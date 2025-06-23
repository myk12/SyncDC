#include "app-all-reduce.h"
#include "spine-leaf.h"
#include "optical-circuit-switching.h"

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
    ns3::LogComponentEnable("AppOCSAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("SyncDCTopologySpineLeafTest", ns3::LOG_INFO);
    ns3::LogComponentEnable("AppRingAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("OCSTest", ns3::LOG_INFO);
    //ns3::LogComponentEnable("BulkSendApplication", ns3::LOG_LOGIC);
    // ns3::LogComponentEnable("TcpSocketBase", ns3::LOG_LOGIC);

    //  parse command line
    std::string topologyConfig = "topology-config.yaml";
    int32_t testbed = 0; // 0 for Spine-Leaf, 1 for OCS test mode, 2 for Fastpass test mode
    std::string linkDelay = "100ns";
    std::string linkBandwidth = "100Gbps";
    uint32_t spineNum = 2;
    uint32_t leafNum = 4;
    uint32_t hostPerLeaf = 8;
    uint32_t serverNum = leafNum * hostPerLeaf;
    ns3::Time reconfTime = ns3::MicroSeconds(100);
    ns3::Time syncErrorTime = ns3::NanoSeconds(100);

    ns3::CommandLine cmd;
    cmd.AddValue("config", "Topology configuration file", topologyConfig);
    cmd.AddValue("testbed", "Testbed mode: 0 for Spine-Leaf, 1 for OCS test mode, 2 for Fastpass test mode", testbed);
    cmd.AddValue("linkDelay", "Link delay", linkDelay);
    cmd.AddValue("linkBandwidth", "Link bandwidth", linkBandwidth);
    // Spine-Leaf Options
    cmd.AddValue("spineNum", "Number of Spine switches", spineNum);
    cmd.AddValue("leafNum", "Number of Leaf switches", leafNum);
    cmd.AddValue("hostPerLeaf", "Number of hosts per leaf switch", hostPerLeaf);
    cmd.AddValue("serverNum", "Number of servers in the testbed", serverNum);
    // OCS Options
    cmd.AddValue("reconfTime", "Reconfiguration time", reconfTime);
    cmd.AddValue("syncErrorTime", "Synchronization error time", syncErrorTime);
    cmd.Parse(argc, argv);

    YAML::Node config = YAML::LoadFile(topologyConfig);
    std::shared_ptr<SyncDCTopologyOCS> topologyOCS = nullptr;
    std::shared_ptr<SyncDCTopologySpineLeaf> topologySpineLeaf = nullptr;
    
    switch (testbed) {
    case 0:
        NS_LOG_INFO("Spine-Leaf test mode enabled");
        // Init Topology
        topologySpineLeaf = std::make_shared<SyncDCTopologySpineLeaf>(config);
        // Init Applications
        InitSpineLeafTest(topologySpineLeaf);
        break;
    case 1:
        NS_LOG_INFO("OCS test mode enabled");
        // Init Topology
        topologyOCS = std::make_shared<SyncDCTopologyOCS>(config);
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
    ns3::Ptr<ns3::FlowMonitor> flowMonitor;
    ns3::FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    flowMonitor->SerializeToXmlFile("allreduce-flow-monitor.xml", true, true);
    return 0;
}
