#include "app-all-reduce.h"
#include "spine-leaf.h"

#include <string>

NS_LOG_COMPONENT_DEFINE("SyncDCML");

int main(int argc, char *argv[]) {
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("SyncDCML", ns3::LOG_ALL);
    ns3::LogComponentEnable("AppAllReduce", ns3::LOG_INFO);
    ns3::LogComponentEnable("SyncDCTopologySpineLeaf", ns3::LOG_INFO);
    //ns3::LogComponentEnable("TcpSocketBase", ns3::LOG_LOGIC);
    // parse command line
    std::string topologyConfig = "topology-config.yaml";
    ns3::CommandLine cmd;
    cmd.AddValue("config", "Topology configuration file", topologyConfig);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Topology configuration file: " << topologyConfig);
    // create topology
    NS_LOG_INFO("Create topology");
    YAML::Node config = YAML::LoadFile(topologyConfig);
    SyncDCTopologySpineLeaf topology(config);

    // Init Application Cluster
    NS_LOG_INFO("Init Server Info for all hosts");
    // Get all leaf-hosts
    std::vector<ns3::NodeContainer> leafHosts = topology.GetLeafNodeHosts();
    // Get all nodes info
    ns3::NodeContainer allHosts;
    std::vector<ns3::Ipv4Address> allHostsIpv4;
    ns3::ApplicationContainer allHostsApps;
    for (auto it = leafHosts.begin(); it != leafHosts.end(); it++)
    {
        allHosts.Add(*it);
        for (uint32_t i = 0; i < (*it).GetN(); i++)
        {
            ns3::Ipv4Address hostIpv4 = topology.GetHostAddress(it - leafHosts.begin(), i);
            NS_LOG_INFO("Host " << hostIpv4 << " is added to all hosts");
            allHostsIpv4.push_back(hostIpv4); 
            // Create all-reduce app
            ns3::Ptr<AppAllReduce> app = ns3::CreateObject<AppAllReduce>();
            ns3::Ptr<ns3::Node> node = (*it).Get(i);
            node->AddApplication(app);
            allHostsApps.Add(app);
        }
    }

    // Init Application Cluster
    NS_LOG_INFO("Init Application Cluster");
    for (uint32_t i = 0; i < allHostsApps.GetN(); i++)
    {
        ns3::Ptr<AppAllReduce> app = allHostsApps.Get(i)->GetObject<AppAllReduce>();
        app->InitClusterInfo(i, allHostsIpv4);
    }

    // Set start time
    allHostsApps.Start(ns3::Seconds(1.0));
    allHostsApps.Stop(ns3::Seconds(10.0));

    // Flow monitor
    ns3::Ptr<ns3::FlowMonitor> flowMonitor;
    ns3::FlowMonitorHelper flowHelper;
    flowMonitor = flowHelper.InstallAll();

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();

    flowMonitor->SerializeToXmlFile("allreduce-flow-monitor.xml", true, true);
    return 0;
}
