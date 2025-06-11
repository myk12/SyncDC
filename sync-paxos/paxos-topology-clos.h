#ifndef PAXOS_TOPOLOGY_CLOS_H
#define PAXOS_TOPOLOGY_CLOS_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"

#include <vector>
#include <string>
#include <unordered_map>

// This is a simple topology for a Clos network

class PaxosTopologyClos {
public:
    PaxosTopologyClos(uint32_t numSpines,
                      uint32_t numLeaves,
                      uint32_t numHostsPerLeaf,
                      std::string bandwidthLeaf2Spine,
                      std::string delayLeaf2Spine,
                      std::string bandwidthHost2Leaf,
                      std::string delayHost2Leaf);
    ~PaxosTopologyClos();

private:
    ns3::NodeContainer m_spineNodes;
    ns3::NodeContainer m_leafNodes;
    std::vector<ns3::NodeContainer> m_hostNodes;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_spineLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_spineLeafInterfaceMatrix;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_hostLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_hostLeafInterfaceMatrix;
};

#endif
