#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <unordered_map>

#include "app-ocs-allreduce.h"

NS_LOG_COMPONENT_DEFINE("OCSTest");

int main(int argc, char *argv[])
{
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("AppOCSAllReduce", ns3::LOG_LEVEL_INFO);
    ns3::LogComponentEnable("OCSTest", ns3::LOG_LEVEL_INFO);

    // Parse command line arguments
    uint32_t numNodes = 10;
    std::string linkBandwidth = "100Gbps";
    std::string linkDelay = "1us";
    uint64_t msgSizeBytes = 1 * 1024 * 1024;
    std::string logDir = "/tmp/";

    ns3::CommandLine cmd;
    cmd.AddValue("numNodes", "Number of nodes", numNodes);
    cmd.AddValue("linkBandwidth", "Link bandwidth", linkBandwidth);
    cmd.AddValue("linkDelay", "Link delay", linkDelay);
    cmd.AddValue("msgSizeBytes", "Message size", msgSizeBytes);
    cmd.AddValue("logDir", "Log directory", logDir);
    cmd.Parse(argc, argv);

    // Show Simulation Parameters
    NS_LOG_INFO("=========== Simulation Parameters ===========");
    NS_LOG_INFO("NumberOCS AllReduce Test");
    NS_LOG_INFO("Number of nodes: " << numNodes);
    NS_LOG_INFO("Link bandwidth: " << linkBandwidth);
    NS_LOG_INFO("Link delay: " << linkDelay);
    NS_LOG_INFO("Message size: " << msgSizeBytes << " bytes");

    // Create nodes
    ns3::NodeContainer servers;
    servers.Create(numNodes);

    // Instal Internet stack
    ns3::InternetStackHelper internet;
    internet.Install(servers);

    // Create point-to-point links
    ns3::PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", ns3::StringValue(linkBandwidth));
    p2p.SetChannelAttribute("Delay", ns3::StringValue(linkDelay));

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");

    std::unordered_map<uint32_t, ns3::Ipv4Address> targetIpMap;
    std::unordered_map<uint32_t, ns3::Ipv4Address> listenIpMap;
    std::unordered_map<uint32_t, ns3::Ipv4Address> bindIpMap;
    for (uint32_t i = 0; i < numNodes; i++)
    {
        ns3::Ipv4InterfaceContainer ifcontain;
        std::string linkName = "p2p-" + std::to_string(i) + "-" + std::to_string(i);
        if (i == numNodes - 1)
        {
            ns3::NetDeviceContainer devices = p2p.Install(servers.Get(i), servers.Get(0));
            // Set the IP addresses
            ifcontain = ipv4.Assign(devices);
            listenIpMap[0] = ifcontain.GetAddress(1);
            targetIpMap[i] = ifcontain.GetAddress(1);
            bindIpMap[i] = ifcontain.GetAddress(0);
        }
        else
        {
            uint32_t j = i + 1;
            ns3::NetDeviceContainer devices = p2p.Install(servers.Get(i), servers.Get(j));
            // Set the IP addresses
            ifcontain = ipv4.Assign(devices);
            listenIpMap[j] = ifcontain.GetAddress(1);
            targetIpMap[i] = ifcontain.GetAddress(1);
            bindIpMap[i] = ifcontain.GetAddress(0);
        }
    }

    // Populate Routing Table
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create OCS AllReduce application
    ns3::ApplicationContainer apps;
    for (uint32_t i = 0; i < numNodes; i++)
    {
        ns3::Ptr<AppOCSAllReduce> app = ns3::CreateObject<AppOCSAllReduce>(
            i,
            targetIpMap[i],
            bindIpMap[i],
            listenIpMap[i],
            msgSizeBytes,
            logDir);
        apps.Add(app);
        servers.Get(i)->AddApplication(app);
    }

    apps.Start(ns3::Seconds(1.0));

    // Run simulation
    ns3::Simulator::Stop(ns3::Seconds(10.0));
    ns3::Simulator::Run();

    // Cleanup
    ns3::Simulator::Destroy();

    NS_LOG_INFO("=========== Simulation Finished ===========");
    return 0;
}
