#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

#include "paxos-app.h"
#include "paxos-frame.h"
#include "paxos-common.h"    
#include "paxos-app-client.h"

// Define Log Component

NS_LOG_COMPONENT_DEFINE("SyncPaxos");

// This is a simple NS-3 simulation that sets up a point-to-point link between two nodes,
// installs a UDP echo server on one node, and a UDP echo client on the other node.
// The client sends a single packet to the server, which echoes it back.

int main(int argc, char *argv[]) {

    ns3::LogComponentEnable("PaxosAppClient", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("SyncPaxos", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosApp", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosFrame", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServerListener", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServerProposer", ns3::LOG_INFO);
    ns3::CommandLine cmd;
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Starting SyncPaxos Simulation");

    // Create 4 nodes, one for client and 3 for servers
    ns3::NodeContainer nodes;
    nodes.Create(4);

    // Create 
    ns3::CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", ns3::StringValue("5Mbps"));
    csma.SetChannelAttribute("Delay", ns3::StringValue("100us"));

    ns3::NetDeviceContainer devices;
    devices = csma.Install(nodes);
    NS_LOG_INFO("Devices installed on nodes");

    ns3::InternetStackHelper stack;
    stack.Install(nodes);
    NS_LOG_INFO("Internet stack installed on nodes");
    // Assign IP addresses to the devices
    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    ns3::Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
    NS_LOG_INFO("IP addresses assigned to devices");

    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    // Install the Paxos application on node 0,1,2
    ns3::ApplicationContainer serverApps;
    NodeInfoList nodeInfoList;
    // Initialize nodeInfoList
    for (uint32_t i = 0; i < nodes.GetN()-1; ++i) {
        ns3::Ptr<ns3::Node> node = nodes.Get(i);
        ns3::Ipv4Address address = interfaces.GetAddress(i);
        nodeInfoList.push_back({i, address, PAXOS_PORT, SERVER_PORT});
    }

    // Create the Paxos application on node 0,1,2
    for (uint32_t i = 0; i < nodes.GetN()-1; ++i) {
        ns3::Ptr<PaxosApp> paxosApp = ns3::CreateObject<PaxosApp>(i, nodeInfoList);

        ns3::Ptr<ns3::Node> node = nodes.Get(i);
        node->AddApplication(paxosApp);
        paxosApp->SetStartTime(ns3::Seconds(0));
        paxosApp->SetStopTime(ns3::Seconds(1.2));
        serverApps.Add(paxosApp);
    }

    // Install the Client application on node 3 (client)
    ns3::ApplicationContainer clientApps;
    ns3::Ptr<ns3::Node> clientNode = nodes.Get(3);
    ns3::Ptr<PaxosAppClient> clientApp = ns3::CreateObject<PaxosAppClient>(nodeInfoList);
    clientNode->AddApplication(clientApp);
    clientApp->SetStartTime(ns3::Seconds(1.0));
    clientApp->SetStopTime(ns3::Seconds(1.2));
    clientApps.Add(clientApp);

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    return 0;
}
