#include "spine-leaf.h"
#include "app-ring-allreduce.h"

NS_LOG_COMPONENT_DEFINE("SyncDCTopologySpineLeafTest");

void InitSpineLeafTest(std::shared_ptr<SyncDCTopologySpineLeaf> topology)
{
    // Init Application Cluster
    NS_LOG_INFO("Init Server Info for all hosts");
    // Get all leaf-hosts
    std::vector<ns3::NodeContainer> leafHosts = topology->GetLeafNodeHosts();
    // Get all nodes info
    ns3::NodeContainer allHosts;
    std::vector<ns3::Ipv4Address> allHostsIpv4;
    ns3::ApplicationContainer allHostsApps;
    for (auto it = leafHosts.begin(); it != leafHosts.end(); it++)
    {
        allHosts.Add(*it);
        for (uint32_t i = 0; i < (*it).GetN(); i++)
        {
            ns3::Ipv4Address hostIpv4 = topology->GetHostAddress(it - leafHosts.begin(), i);
            NS_LOG_INFO("Host " << hostIpv4 << " is added to all hosts");
            allHostsIpv4.push_back(hostIpv4);
        }
    }

    // Install AllReduce Application
    for (uint32_t i = 0; i < allHostsIpv4.size(); i++)
    {
        ns3::Ptr<ns3::Node> node = allHosts.Get(i);
        ns3::Ptr<AppRingAllReduce> app = ns3::CreateObject<AppRingAllReduce>(i, allHostsIpv4);

        node->AddApplication(app);
        allHostsApps.Add(app);
    }

    // Set start time
    allHostsApps.Start(ns3::Seconds(0.0));
    allHostsApps.Stop(ns3::Seconds(10.0));
}