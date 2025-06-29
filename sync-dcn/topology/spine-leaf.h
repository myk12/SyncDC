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

typedef struct SpineLeafTopologyConfig {
    uint32_t numSpines;          // Number of spine switches
    uint32_t numLeaves;          // Number of leaf switches
    uint32_t numHostsPerLeaf;    // Number of hosts per leaf switch
    std::string linkBandwidth;   // Bandwidth of the links
    std::string linkDelay;       // Delay of the links
    uint64_t msgSize;          // Size of the message to sond in bytes 
    std::string logDir;       // Directory for log files
} SpineLeafTopologyConfig;

// This is a simple topology for a Clos network
class SyncDCTopologySpineLeaf {
public:
    SyncDCTopologySpineLeaf(SpineLeafTopologyConfig& config);
    ~SyncDCTopologySpineLeaf();

    ns3::Ipv4Address GetSpineAddress(uint32_t spineId);
    ns3::Ipv4Address GetLeafAddress(uint32_t spineId, uint32_t leafId);
    ns3::Ipv4Address GetHostAddress(uint32_t leafId, uint32_t hostId);
    uint64_t GetMsgSize();
    std::string GetLogDir();
    std::string GetLinkBandwidth();
    uint32_t GetNumSpines();
    uint32_t GetNumLeaves();
    uint32_t GetNumHostsPerLeaf();

    std::vector<ns3::NodeContainer> GetLeafNodeHosts();

    void ExportTopologyToYaml(const std::string& filename);

    void BindServerId2NodeId(uint32_t serverId, uint32_t nodeId);
    uint32_t GetLeafIdbyServerId(uint32_t serverId);
    ns3::Ptr<ns3::Node> GetNodeByServerId(uint32_t serverId);
    ns3::Ptr<ns3::Node> GetSpineNode(uint32_t spineId);
    ns3::Ptr<ns3::Node> GetLeafNode(uint32_t leafId);

private:
    ns3::NodeContainer m_spineNodes;
    ns3::NodeContainer m_leafNodes;
    std::vector<ns3::NodeContainer> m_hostNodes;
    uint64_t m_msgSize; // Size of the message to send in bytes

    SpineLeafTopologyConfig m_config;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_spineLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_spineLeafInterfaceMatrix;

    std::vector<std::vector<ns3::NetDeviceContainer>> m_hostLeafLinksMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_hostLeafInterfaceMatrix;

    std::map<uint32_t, uint32_t> m_serverId2NodeIdMap;
    std::map<uint32_t, uint32_t> m_nodeId2LeafIdMap;
    std::map<uint32_t, uint32_t> m_serverId2LeafIdMap;
    std::map<uint32_t, ns3::Ptr<ns3::Node>> m_serverId2NodeMap;
};

#endif
