#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"

#include "paxos-app-server.h"
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

    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_TIME);
    ns3::LogComponentEnableAll(ns3::LOG_PREFIX_NODE);
    ns3::LogComponentEnable("PaxosAppClient", ns3::LOG_DEBUG);
    ns3::LogComponentEnable("SyncPaxos", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServer", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosFrame", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServerListener", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosAppServerProposer", ns3::LOG_INFO);
    ns3::LogComponentEnable("PaxosTopologyClos", ns3::LOG_INFO);

    ns3::CommandLine cmd;
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Starting SyncPaxos Simulation");
    uint32_t numSpine = 3;
    uint32_t numLeaf = 5;
    uint32_t numHostsPerLeaf = 5;
    std::string bandwidthLeaf2Spine = "1Gbps";
    std::string delayLeaf2Spine = "5us";
    std::string bandwidthHost2Leaf = "1Gbps";
    std::string delayHost2Leaf = "5us";

    // Define the topology
    NS_LOG_INFO("Creating Clos topology with " << numSpine << " spines, " << numLeaf << " leaves, " << numHostsPerLeaf << " hosts per leaf, " << bandwidthLeaf2Spine << " bandwidth leaf to spine, " << delayLeaf2Spine << " delay leaf to spine, " << bandwidthHost2Leaf << " bandwidth host to leaf, " << delayHost2Leaf << " delay host to leaf");
    PaxosTopologyClos topology(
        numSpine,
        numLeaf,
        numHostsPerLeaf,
        bandwidthLeaf2Spine,
        delayLeaf2Spine,
        bandwidthHost2Leaf,
        delayHost2Leaf);

    // Init Paxos Server Cluster
    NS_LOG_INFO("Init Paxos Server Cluster");
    std::vector<std::pair<uint32_t, uint32_t>> hostIdList;
    for (uint32_t i = 0; i < numLeaf; i++) {
        hostIdList.push_back(std::make_pair(i, 0));
    }

    int32_t ret = topology.InitPaxosServerCluster(hostIdList);
    if (ret != 0) {
        NS_LOG_ERROR("Init Paxos Server Cluster failed");
        return -1;
    }

    // Init Paxos Client Cluster
    NS_LOG_INFO("Init Paxos Client Cluster");
    std::vector<uint32_t> spineIdList;
    spineIdList.push_back(numSpine/2);
    ret = topology.InitPaxosClientCluster(spineIdList);
    if (ret != 0) {
        NS_LOG_ERROR("Init Paxos Client Cluster failed");
        return -1;
    }

    // Set Paxos Server App Start Stop
    ns3::Time start = ns3::Seconds(1.0);
    ns3::Time end = ns3::Seconds(1.2);
    topology.SetPaxosServerAppStartStop(start, end);
    topology.SetPaxosClientAppStartStop(start, end);

    // Run the simulation
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
    return 0;
}
