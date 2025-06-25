#include "app-all-reduce.h"
#include "spine-leaf.h"
#include "optical-circuit-switching.h"
#include "ml-common.h"

#include <string>

NS_LOG_COMPONENT_DEFINE("SyncDCML");

extern void InitOCSTest(std::shared_ptr<SyncDCTopologyOCS> topology);
extern void InitSpineLeafTest(std::shared_ptr<SyncDCTopologySpineLeaf> topology);

int main(int argc, char *argv[])
{
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("SyncDCML", ns3::LOG_ALL);
    ns3::LogComponentEnable("AppAllReduce", ns3::LOG_INFO);
    //ns3::LogComponentEnable("AppOCSAllReduce", ns3::LOG_INFO);
    //ns3::LogComponentEnable("SyncDCTopologySpineLeafTest", ns3::LOG_INFO);
    //ns3::LogComponentEnable("AppRingAllReduce", ns3::LOG_INFO);
    //ns3::LogComponentEnable("OCSTest", ns3::LOG_INFO);
    //ns3::LogComponentEnable("BulkSendApplication", ns3::LOG_LOGIC);
    //ns3::LogComponentEnable("TcpSocketBase", ns3::LOG_LOGIC);

    //  parse command line
    std::string topologyConfig = "topology-config.yaml";
    int32_t testbed = 0; // 0 for Spine-Leaf, 1 for OCS test mode, 2 for Fastpass test mode
    std::string linkDelay = "1us";
    std::string linkBandwidth = "100Gbps";
    uint32_t spineNum = 2;
    uint32_t leafNum = 4;
    uint32_t hostPerLeaf = 8;
    uint32_t serverNum = leafNum * hostPerLeaf;
    ns3::Time reconfTime = ns3::MicroSeconds(100);
    ns3::Time syncErrorTime = ns3::NanoSeconds(100);
    std::string msgSizeStr = "2MB";
    std::string logDir = "/tmp/";   // We need the log directory to parallelize the batch test

    ns3::CommandLine cmd;
    cmd.AddValue("config", "Topology configuration file", topologyConfig);
    cmd.AddValue("testbed", "Testbed mode: 0 for Spine-Leaf, 1 for OCS test mode, 2 for Fastpass test mode", testbed);
    cmd.AddValue("linkDelay", "Link delay", linkDelay);
    cmd.AddValue("linkBandwidth", "Link bandwidth", linkBandwidth);
    cmd.AddValue("logDir", "Log directory", logDir);
    // Spine-Leaf Options
    cmd.AddValue("spineNum", "Number of Spine switches", spineNum);
    cmd.AddValue("leafNum", "Number of Leaf switches", leafNum);
    cmd.AddValue("hostPerLeaf", "Number of hosts per leaf switch", hostPerLeaf);
    cmd.AddValue("serverNum", "Number of servers in the testbed", serverNum);
    // OCS Options
    cmd.AddValue("reconfTime", "Reconfiguration time", reconfTime);
    cmd.AddValue("syncErrorTime", "Synchronization error time", syncErrorTime);
    cmd.AddValue("msgSize", "Message size: MB or KB", msgSizeStr);
    cmd.Parse(argc, argv);
    // Convert message size from string to bytes
    uint64_t msgSize = DataSizeStr2Bytes(msgSizeStr);

    // Show the Current Configuration
    NS_LOG_INFO("=========== Simulation Configuration ===========");
    NS_LOG_INFO("Topology Configuration File: " << topologyConfig);
    switch (testbed) {
    case 0:
        NS_LOG_INFO("Testbed Mode: Spine-Leaf");
        NS_LOG_INFO("Number of Spine switches: " << spineNum);
        NS_LOG_INFO("Number of Leaf switches: " << leafNum);
        NS_LOG_INFO("Number of Hosts per Leaf switch: " << hostPerLeaf);
        NS_LOG_INFO("Number of Servers in the testbed: " << serverNum);
        break;
    case 1:
        NS_LOG_INFO("Testbed Mode: OCS");
        NS_LOG_INFO("Number of Nodes in the testbed: " << serverNum);
        NS_LOG_INFO("Reconfiguration time: " << reconfTime);
        NS_LOG_INFO("Synchronization error time: " << syncErrorTime);
        break;
    case 2:
        NS_LOG_INFO("Testbed Mode: Fastpass");
        NS_LOG_INFO("Number of Servers in the testbed: " << serverNum);
        break;
    default:
        NS_LOG_ERROR("Invalid testbed mode");
        return 1;
    }
    NS_LOG_INFO("Link Delay: " << linkDelay);
    NS_LOG_INFO("Link Bandwidth: " << linkBandwidth);
    NS_LOG_INFO("Message Size: " << msgSizeStr << " (" << msgSize << " bytes)");
    NS_LOG_INFO("===============================================");

    std::shared_ptr<SyncDCTopologyOCS> topologyOCS = nullptr;
    std::shared_ptr<SyncDCTopologySpineLeaf> topologySpineLeaf = nullptr;
    OCSTopologyConfig ocsConfig;
    SpineLeafTopologyConfig spineLeafConfig;

    // Set topology configuration based on command line arguments
    ocsConfig.numNodes = serverNum; // Number of nodes in the testbed
    ocsConfig.linkBandwidth = linkBandwidth;
    ocsConfig.linkDelay = linkDelay;
    ocsConfig.reConfTime = reconfTime;
    ocsConfig.syncErrorTime = syncErrorTime;
    ocsConfig.msgSize = msgSize;
    ocsConfig.logDir = logDir;

    spineLeafConfig.numSpines = spineNum;
    spineLeafConfig.numLeaves = leafNum;
    spineLeafConfig.numHostsPerLeaf = hostPerLeaf;
    spineLeafConfig.linkBandwidth = linkBandwidth;
    spineLeafConfig.linkDelay = linkDelay;
    spineLeafConfig.msgSize = msgSize;
    spineLeafConfig.logDir = logDir;
    
    switch (testbed) {
    case 0:
        NS_LOG_INFO("Spine-Leaf test mode enabled");
        // Init Topology
        topologySpineLeaf = std::make_shared<SyncDCTopologySpineLeaf>(spineLeafConfig);
        // Init Applications
        InitSpineLeafTest(topologySpineLeaf);
        break;
    case 1:
        NS_LOG_INFO("OCS test mode enabled");
        // Init Topology
        topologyOCS = std::make_shared<SyncDCTopologyOCS>(ocsConfig);
        // Init Applications
        InitOCSTest(topologyOCS);
        break;
    case 2:
        NS_LOG_INFO("Fastpass test mode enabled");
        break;
    default:
        NS_LOG_ERROR("Invalid testbed mode");
        break;
    }

    // Flow monitor
    NS_LOG_INFO("Install Flow Monitor");
    ns3::Ptr<ns3::FlowMonitor> flowMonitor;
    ns3::FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    // Run simulation
    NS_LOG_INFO("Start simulation");
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    flowMonitor->SerializeToXmlFile("allreduce-flow-monitor.xml", true, true);
    NS_LOG_INFO("Simulation finished");
    return 0;
}
