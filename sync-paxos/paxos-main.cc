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
#include "paxos-topology-clos.h"

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
    ns3::LogComponentEnable("PaxosTopologyClos", ns3::LOG_INFO);
    ns3::CommandLine cmd;
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Starting SyncPaxos Simulation");

    // Define the topology
    PaxosTopologyClos topology(
        3,  // Number of Spines
        5,  // Number of Leaves
        5,  // Number of Hosts per Leaf
        "1Gbps", // Bandwidth Leaf to Spine
        "10ms", // Delay Leaf to Spine
        "1Gbps", // Bandwidth Host to Leaf  
        "10ms"  // Delay Host to Leaf
    );

    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    return 0;
}
