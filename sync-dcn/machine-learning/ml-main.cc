#include "app-all-reduce.h"
#include "spine-leaf.h"
#include "optical-circuit-switching.h"

#include <string>

NS_LOG_COMPONENT_DEFINE("SyncDCML");

bool testOCS = true;
extern void InitOCSTest(std::shared_ptr<SyncDCTopologyOCS> topology);
extern void InitSpineLeafTest(std::shared_ptr<SyncDCTopologySpineLeaf> topology);

int main(int argc, char *argv[])
{
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("SyncDCML", ns3::LOG_ALL);
    ns3::LogComponentEnable("AppAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("SyncDCTopologySpineLeafTest", ns3::LOG_INFO);
    ns3::LogComponentEnable("AppRingAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("OCSTest", ns3::LOG_INFO);
    //ns3::LogComponentEnable("BulkSendApplication", ns3::LOG_LOGIC);
    // ns3::LogComponentEnable("TcpSocketBase", ns3::LOG_LOGIC);
    //  parse command line
    std::string topologyConfig = "topology-config.yaml";
    ns3::CommandLine cmd;
    cmd.AddValue("config", "Topology configuration file", topologyConfig);
    cmd.Parse(argc, argv);

    YAML::Node config = YAML::LoadFile(topologyConfig);
    std::shared_ptr<SyncDCTopologyOCS> topologyOCS = nullptr;
    std::shared_ptr<SyncDCTopologySpineLeaf> topologySpineLeaf = nullptr;
    if (testOCS)
    {
        NS_LOG_INFO("OCS test mode enabled");
        // Init Topology
        topologyOCS = std::make_shared<SyncDCTopologyOCS>(config);
        // Init Applications
        InitOCSTest(topologyOCS);
    }
    else
    {
        NS_LOG_INFO("Spine-Leaf test mode enabled");
        topologySpineLeaf = std::make_shared<SyncDCTopologySpineLeaf>(config);
        InitSpineLeafTest(topologySpineLeaf);
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
