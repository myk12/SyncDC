#include <iostream>
#include <queue>
#include <vector>
#include <cstdint>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

#include "spine-leaf.h"
#include "app-ring-allreduce.h"

NS_LOG_COMPONENT_DEFINE("SpineLeafTest");

// Custom comparator for min-heap based on pair.second (job_count)
struct Compare
{
    bool operator()(const std::pair<uint32_t, uint32_t> &a, const std::pair<uint32_t, uint32_t> &b) const
    {
        return a.second > b.second || (a.second == b.second && a.first > b.first);
    }
};

int main(int argc, char *argv[])
{
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("SpineLeafTest", ns3::LOG_INFO);
    ns3::LogComponentEnable("SyncDCTopologySpineLeaf", ns3::LOG_INFO);
    ns3::LogComponentEnable("AppRingAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("FastPassHeader", ns3::LOG_DEBUG);
    

    // Parse command line parameters
    std::string workloadFile = "workload.csv";
    SpineLeafTopologyConfig config;

    // Default value
    config.logDir = "/tmp/ring/";
    config.numSpines = 2;
    config.numLeaves = 4;
    config.numHostsPerLeaf = 8;
    config.linkBandwidth = "100Gbps";
    config.linkDelay = "1us";
    config.msgSize = 2 * 1024 * 1024;

    ns3::CommandLine cmd;
    cmd.AddValue("logDir", "Log directory", config.logDir);
    cmd.AddValue("numSpines", "Number of Spines", config.numSpines);
    cmd.AddValue("numLeaves", "Number of Leaves", config.numLeaves);
    cmd.AddValue("numHostsPerLeaf", "Number of Hosts per Leaf", config.numHostsPerLeaf);
    cmd.AddValue("linkBandwidth", "Link Bandwidth", config.linkBandwidth);
    cmd.AddValue("linkDelay", "Link Delay", config.linkDelay);
    cmd.AddValue("workloadFile", "Workload file", workloadFile);
    cmd.Parse(argc, argv);

    // Show Simulation Parameters
    NS_LOG_INFO("============= Simulation Parameters =============");
    NS_LOG_INFO("Topolgoy : Spine-Leaf");
    NS_LOG_INFO("Log Dir: " << config.logDir);
    NS_LOG_INFO("Number of Spines: " << config.numSpines);
    NS_LOG_INFO("Number of Leaves: " << config.numLeaves);
    NS_LOG_INFO("Number of Hosts per Leaf: " << config.numHostsPerLeaf);
    NS_LOG_INFO("Link Bandwidth: " << config.linkBandwidth);
    NS_LOG_INFO("Link Delay: " << config.linkDelay);
    NS_LOG_INFO("===============================================");

    // Init Topology
    NS_LOG_INFO("Init Topology");
    std::shared_ptr<SyncDCTopologySpineLeaf> topology = std::make_shared<SyncDCTopologySpineLeaf>(config);

    // Read workload file and shedule workload
    // file format, CSV: jobid,request_nodes,create_time,data_size
    std::priority_queue<std::pair<uint32_t, uint32_t>,
                        std::vector<std::pair<uint32_t, uint32_t>>,
                        Compare>
        jobsQueue;
    // Init queue
    NS_LOG_INFO("Init Job Queue");
    uint32_t serverNum = config.numLeaves * config.numHostsPerLeaf;
    for (uint32_t i = 0; i < serverNum; i++)
    {
        jobsQueue.push(std::make_pair(i, 0));
    }

    // Read workload file
    NS_LOG_INFO("Read Workload File");
    std::ifstream workloadFileStream(workloadFile);
    std::string line;
    uint16_t servicePort = 9000;
    while (std::getline(workloadFileStream, line))
    {
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, ','))
        {
            tokens.push_back(token);
        }

        uint32_t jobId = std::stoi(tokens[0]);        // Job Id
        uint32_t requestNodes = std::stoi(tokens[1]); // Number of nodes this job needed
        uint64_t createTime = std::stoi(tokens[2]);   // Job creating time with nanosecond
        uint64_t dataSize = std::stoull(tokens[3]);   // Data size in MBytes
        NS_LOG_INFO("Job " << jobId << " is sheduled at " << createTime << " with " << requestNodes << " nodes and " << dataSize << " bytes");

        // Assign server to this job
        std::vector<uint32_t> serverList;
        for (uint32_t i = 0; i < requestNodes; i++)
        {
            // get server with least load from queue
            std::pair<uint32_t, uint32_t> job = jobsQueue.top();
            jobsQueue.pop();
            serverList.push_back(job.first);
            job.second++;
            jobsQueue.push(job);
        }

        // Init AppRingAllReduce for this job
        // Get all nodes info
        NS_LOG_INFO("Init AppRingAllReduce for job " << jobId << " with " << serverList.size() << " servers");
        ns3::NodeContainer selectedNodes;
        std::vector<std::pair<uint32_t, ns3::Ipv4Address>> selectedServerIdIpv4;
        ns3::ApplicationContainer selectedNodesApps;
        for (uint32_t i = 0; i < serverList.size(); i++)
        {
            uint32_t serverId = serverList[i];
            ns3::Ptr<ns3::Node> node = topology->GetNodeByServerId(serverId);
            ns3::Ipv4Address serverIpv4 = node->GetObject<ns3::Node>()->GetObject<ns3::Ipv4>()->GetAddress(1, 0).GetLocal();
            selectedNodes.Add(node);
            selectedServerIdIpv4.push_back(std::make_pair(serverId, serverIpv4));
            // NS_LOG_INFO("Server " << serverId << " is added to selected nodes");
        }

        // Install AllReduce Application
        std::string logPath = topology->GetLogDir() + "/job_" + std::to_string(jobId) + ".log";
        NS_LOG_INFO("Install AllReduce Application");
        for (uint32_t i = 0; i < requestNodes; i++)
        {
            ns3::Ptr<AppRingAllReduce> app = ns3::CreateObject<AppRingAllReduce>(i, selectedServerIdIpv4,
                                                                                 servicePort, // for each round, we have different port
                                                                                  topology->GetMsgSize(),
                                                                                  jobId,
                                                                                  logPath);
            selectedNodes.Get(i)->AddApplication(app);
            selectedNodesApps.Add(app);
        }

        // Set start time
        servicePort++;
        selectedNodesApps.Start(ns3::NanoSeconds(createTime));
    }

    NS_LOG_INFO("Init Topology Done");

    // Flow monitor
    NS_LOG_INFO("Init Flow Monitor");
    ns3::Ptr<ns3::FlowMonitor> flowMonitor;
    ns3::FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Run simulation
    NS_LOG_INFO("Run Simulation");
    ns3::Simulator::Run();
    NS_LOG_INFO("Run Simulation Done");
    ns3::Simulator::Destroy();

    flowMonitor->SerializeToXmlFile("spine-leaf-ml-workload.xml", true, true);
    NS_LOG_INFO("Simulation Finished");
    return 0;
}
