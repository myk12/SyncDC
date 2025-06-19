#ifndef TOPOLOGY_SPINE_LEAF_H
#define TOPOLOGY_SPINE_LEAF_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/flow-monitor-module.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

// This is a simple topology for a Clos network
class SyncDCTopologySpineLeaf {
public:
    SyncDCTopologySpineLeaf(YAML::Node& config);
    ~SyncDCTopologySpineLeaf();

    ns3::Ipv4Address GetSpineAddress(uint32_t spineId);
    ns3::Ipv4Address GetLeafAddress(uint32_t spineId, uint32_t leafId);
    ns3::Ipv4Address GetHostAddress(uint32_t leafId, uint32_t hostId);

    std::vector<ns3::NodeContainer> GetLeafNodeHosts();

    void ExportTopologyToYaml(const std::string& filename);

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
