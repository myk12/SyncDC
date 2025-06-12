#ifndef PAXOS_TOPOLOGY_CLOS_H
#define PAXOS_TOPOLOGY_CLOS_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"

#include "paxos-common.h"
#include "paxos-app-server.h"
#include "paxos-app-client.h"

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
                      std::string delayHost2Leaf,
                      PaxosConfig paxosConfig);
    ~PaxosTopologyClos();

    ns3::Ipv4Address GetSpineAddress(uint32_t spineId);
    ns3::Ipv4Address GetLeafAddress(uint32_t spineId, uint32_t leafId);
    ns3::Ipv4Address GetHostAddress(uint32_t leafId, uint32_t hostId);

    int32_t InitPaxosServerCluster(std::vector<std::pair<uint32_t, uint32_t>> hostIdList);
    // Usually, the client is installed on the spine layer
    // and there is only one client
    int32_t InitPaxosClientCluster(std::vector<uint32_t> spineIdList);

    void SetPaxosServerAppStartStop(ns3::Time start, ns3::Time end);
    void SetPaxosClientAppStartStop(ns3::Time start, ns3::Time end);

private:
    ns3::NodeContainer m_spineNodes;
    ns3::NodeContainer m_leafNodes;
    std::vector<ns3::NodeContainer> m_hostNodes;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_spineLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_spineLeafInterfaceMatrix;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_hostLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_hostLeafInterfaceMatrix;

    NodeInfoList  m_serverInfoList;
    
    ns3::ApplicationContainer m_paxosAppServerContainer;
    ns3::ApplicationContainer m_paxosAppClientContainer;

    PaxosConfig m_paxosConfig;
};

#endif
