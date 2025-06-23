#ifndef TOPOLOGY_OPTICAL_CIRCUIT_SWITCHING
#define TOPOLOGY_OPTICAL_CIRCUIT_SWITCHING

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <yaml-cpp/yaml.h>
#include <vector>

typedef std::vector<std::vector<bool>> CircuitMatrix;

class SyncDCTopologyOCS {
public:
    SyncDCTopologyOCS(YAML::Node& config);
    ~SyncDCTopologyOCS();

    ns3::NodeContainer GetNodes();
    ns3::Ipv4Address GetNodeAddr(uint32_t nodeId);
    ns3::Ipv4Address GetNodeAddr(uint32_t nodeId, uint32_t targetId);
    
    std::vector<CircuitMatrix> GetCircuitMatrix();
    ns3::Time GetReConfTime();
    ns3::Time GetSyncErrorTime();
    std::string GetLinkBandwidth();
    std::string GetLinkDelay();
    std::map<ns3::Ipv4Address, uint32_t> GetMapAddr2Id();

    void GenerateCircuitMatrix();
private:
    uint32_t m_numNodes;
    ns3::Time m_reConfTime;
    ns3::Time m_syncErrorTime;
    ns3::NodeContainer m_Nodes;
    std::string m_linkBandwidth;
    std::string m_linkDelay;
    std::vector<std::vector<ns3::NetDeviceContainer>> m_linkMatrix;
    std::vector<std::vector<ns3::Ipv4InterfaceContainer>> m_ipMatrix;
    std::vector<CircuitMatrix> m_circuitMatrix;
    std::map<uint32_t, std::map<uint32_t, ns3::Ipv4Address>> m_MapId2Addr;
    std::map<ns3::Ipv4Address, uint32_t> m_MapAddr2Id;
};

#endif

