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

    // Get All Server Addr Info
    NS_LOG_INFO("Get All Server Addr Info");
    std::vector<ns3::Ipv4Address> serverAddrs;
    serverAddrs.reserve(nodes.GetN());
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        serverAddrs.push_back(topology->GetNodeAddr(i));
    }

    // Install AllReduce Application
    NS_LOG_INFO("Install AllReduce Application");
    for (uint32_t i = 0; i < nodes.GetN(); i++) {
        ns3::Ptr<AppOCSAllReduce> app = ns3::CreateObject<AppOCSAllReduce>(
            i, serverAddrs, 
            topology->GetLinkDelay(), 
            topology->GetLinkBandwidth(), 
            topology->GetReConfTime(), 
            topology->GetSyncErrorTime());
        
        app->SetStartTime(ns3::Seconds(0.0));
        nodes.Get(i)->AddApplication(app);
    }

    NS_LOG_INFO("Init OCS Test Done");
}
