#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

#include "app-ocs-allreduce.h"
#include "optical-circuit-switching.h"

NS_LOG_COMPONENT_DEFINE("OCSTest");

void
InitOCSTest(std::shared_ptr<SyncDCTopologyOCS> topology)
{
    // Init Applications
    NS_LOG_INFO("Init OCS Test");
    ns3::NodeContainer nodes = topology->GetNodes();

    // Install AllReduce Application
    NS_LOG_INFO("Install AllReduce Application");
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        std::vector<ns3::Ipv4Address> serversAddrlist;
        // Get target addresses for this server
        for (uint32_t j = 0; j < nodes.GetN(); j++)
        {
            //NS_LOG_INFO("Server " << i << " will connect to server " << j);
            serversAddrlist.push_back(topology->GetNodeAddr(j, i));
        }

        NS_LOG_INFO("Creating AppOCSAllReduce for server " << i << "");
        ns3::Ptr<AppOCSAllReduce> app = ns3::CreateObject<AppOCSAllReduce>(
            i, serversAddrlist,
            topology->GetLinkDelay(), 
            topology->GetLinkBandwidth(), 
            topology->GetReConfTime(), 
            topology->GetSyncErrorTime(),
            topology->GetMapAddr2Id(),
            topology->GetMsgSize());
        
        app->SetStartTime(ns3::Seconds(0.0));
        nodes.Get(i)->AddApplication(app);
    }

    NS_LOG_INFO("Init OCS Test Done");
}
