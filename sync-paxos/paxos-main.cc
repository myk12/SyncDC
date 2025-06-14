#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

#include "paxos-app-server.h"
#include "paxos-frame.h"
#include "paxos-common.h"
#include "paxos-app-client.h"
#include "paxos-topology-clos.h"

// Define Log Component

NS_LOG_COMPONENT_DEFINE("SyncPaxos");

// This is a simple NS-3 simulation that sets up a point-to-point link between two nodes,
// installs a UDP echo server on one node, and a UDP echo client on the other node.
// The client sends a single packet to the server, which echoes it back.

PaxosConfig g_paxosConfig;

int main(int argc, char *argv[])
{

    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("PaxosAppClient", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("SyncPaxos", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServer", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosFrame", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("PaxosAppServerListener", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("PaxosAppServerProposer", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("PaxosTopologyClos", ns3::LOG_INFO);

    ns3::CommandLine cmd;

    // The configuration file has the lower priority than the command line arguments.
    cmd.AddValue("config", "Path to the configuration file.", g_paxosConfig.configFilePath);
    cmd.AddValue("sync", "Set Paxos execution to synchronous (true) or asynchronous (false). Default is true (synchronous).", g_paxosConfig.isSynchronous);

    // 2. Network parameters
    //    for synchronous mode
    cmd.AddValue("clockSyncError", "Clock synchronization error for synchronous mode (e.g., '10ns', '1us').", g_paxosConfig.clockSyncError);
    cmd.AddValue("boundedMessageDelay", "Bounded message delay for synchronous mode (e.g., '5ms', '10ms').", g_paxosConfig.boundedMessageDelay);

    //    for asynchronous mode
    cmd.AddValue("linkDelay", "Link delay for asynchronous mode (e.g., '10ms', '50ms').", g_paxosConfig.linkDelay);
    cmd.AddValue("lossRate", "Packet loss rate for asynchronous mode (e.g., 0.01 for 1%).", g_paxosConfig.packetLossRate);

    // 3. Node failure rate
    cmd.AddValue("failureRate", "Node failure rate (e.g., 0.05 for 5%).", g_paxosConfig.nodeFailureRate);

    cmd.Parse(argc, argv);

    // Output Configuration
    NS_LOG_INFO("-------- Configuration: --------");
    NS_LOG_INFO("Config File Path: " << g_paxosConfig.configFilePath);
    NS_LOG_INFO("Synchronous: " << g_paxosConfig.isSynchronous);
    NS_LOG_INFO("Clock Sync Error: " << g_paxosConfig.clockSyncError);
    NS_LOG_INFO("Bounded Message Delay: " << g_paxosConfig.boundedMessageDelay);

    NS_LOG_INFO("Starting SyncPaxos Simulation");
    uint32_t numSpine = 3;
    uint32_t numLeaf = 5;
    uint32_t numHostsPerLeaf = 5;

    std::string bandwidthLeaf2Spine = "1Gbps";
    std::string delayLeaf2Spine = "5us";
    std::string bandwidthHost2Leaf = "1Gbps";
    std::string delayHost2Leaf = "5us";

    if (g_paxosConfig.isSynchronous)
    {
        // In synchronous way, we set the delay as little as possible
        // so that we can make sure the msg delay is bounded.
        delayLeaf2Spine = "5us";
        delayHost2Leaf = "5us";
    }
    else
    {
        // In asynchronous way, we set the delay as the specified value.
        // Since the hop is 4, so we set the delay as 1/4 of the specified value.
        uint32_t delay = ns3::Time(g_paxosConfig.linkDelay).GetNanoSeconds() / 4;
        delayLeaf2Spine = std::to_string(delay) + "ns";
        delayHost2Leaf = std::to_string(delay) + "ns";
    }

    // Define the topology
    NS_LOG_INFO("Creating Clos topology with " << numSpine << " spines, " << numLeaf << " leaves, " << numHostsPerLeaf << " hosts per leaf, " << bandwidthLeaf2Spine << " bandwidth leaf to spine, " << delayLeaf2Spine << " delay leaf to spine, " << bandwidthHost2Leaf << " bandwidth host to leaf, " << delayHost2Leaf << " delay host to leaf");
    PaxosTopologyClos topology(
        numSpine,
        numLeaf,
        numHostsPerLeaf,
        bandwidthLeaf2Spine,
        delayLeaf2Spine,
        bandwidthHost2Leaf,
        delayHost2Leaf,
        g_paxosConfig);

    // Init Paxos Server Cluster
    NS_LOG_INFO("Init Paxos Server Cluster");
    std::vector<std::pair<uint32_t, uint32_t>> hostIdList;
    for (uint32_t i = 0; i < numLeaf; i++)
    {
        hostIdList.push_back(std::make_pair(i, 0));
    }

    int32_t ret = topology.InitPaxosServerCluster(hostIdList);
    if (ret != 0)
    {
        NS_LOG_ERROR("Init Paxos Server Cluster failed");
        return -1;
    }

    // Init Paxos Client Cluster
    NS_LOG_INFO("Init Paxos Client Cluster");
    std::vector<uint32_t> spineIdList;
    spineIdList.push_back(numSpine / 2);
    ret = topology.InitPaxosClientCluster(spineIdList);
    if (ret != 0)
    {
        NS_LOG_ERROR("Init Paxos Client Cluster failed");
        return -1;
    }

    // Set Paxos Server App Start Stop
    ns3::Time start = ns3::Seconds(1.0);
    ns3::Time end = ns3::Seconds(2.0);
    topology.SetPaxosServerAppStartStop(start, end);
    topology.SetPaxosClientAppStartStop(start, end);

    // Run the simulation
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    return 0;
}
