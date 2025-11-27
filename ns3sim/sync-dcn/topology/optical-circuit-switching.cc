#include "optical-circuit-switching.h"

// Now the NS-3 do not have a module for optical circuit switch.
// So we need to implement it by ourselves.
// This is a simple optical circuit switch that connect
// all nodes with point-to-point links.

// In the future, we can implement a network device model
// that simulates the optical circuit switch.

NS_LOG_COMPONENT_DEFINE("SyncDCTopologyOCS");

SyncDCTopologyOCS::SyncDCTopologyOCS(OCSTopologyConfig &config)
{
    NS_LOG_INFO("Creating Optical Circuit Switch topology");
    m_config = config;
    // Get total servers
    m_reConfTime = config.reConfTime;
    m_syncErrorTime = config.syncErrorTime;
    m_linkBandwidth = config.linkBandwidth;
    m_linkDelay = config.linkDelay;
    m_numNodes = config.numNodes;
    NS_LOG_INFO("Number of nodes: " << m_numNodes);
    // Create Nodes and Connect them with Point-to-Point Links
    m_Nodes.Create(m_numNodes);

    // Install Internet Stack
    ns3::InternetStackHelper stack;
    stack.Install(m_Nodes);

    // Create Point-to-Point Links
    ns3::PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", ns3::StringValue(m_linkBandwidth));
    p2p.SetChannelAttribute("Delay", ns3::StringValue(m_linkDelay));

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.0.0.0");

    for (uint32_t i = 0; i < m_numNodes; i++)
    {
        std::vector<ns3::NetDeviceContainer> linkArray;
        std::vector<ns3::Ipv4InterfaceContainer> ipArray;
        std::vector<ns3::Ipv4Address> nodeAddrArray;
        for (uint32_t j = i + 1; j < m_numNodes; j++)
        {
            // Install P2P links between nodes
            ns3::NetDeviceContainer link = p2p.Install(m_Nodes.Get(i), m_Nodes.Get(j));
            linkArray.push_back(link);

            // Assign IP addresses to the devices
            ns3::Ipv4InterfaceContainer ip = ipv4.Assign(link);
            ipArray.push_back(ip);
            // Store the IP addresses for later use
            m_MapId2Addr[i][j] = ip.GetAddress(0);
            m_MapId2Addr[j][i] = ip.GetAddress(1);
            m_MapId2Addr[i][i] = ip.GetAddress(0); // Self address
            m_MapId2Addr[j][j] = ip.GetAddress(1); // Self address
            m_MapAddr2Id[ip.GetAddress(0)] = i;
            m_MapAddr2Id[ip.GetAddress(1)] = j;
        }

        m_linkMatrix.push_back(linkArray);
        m_ipMatrix.push_back(ipArray);
    }

    // Populate Routing Table
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

SyncDCTopologyOCS::~SyncDCTopologyOCS()
{
    NS_LOG_INFO("Destroying Optical Circuit Switch topology");
}

ns3::NodeContainer
SyncDCTopologyOCS::GetNodes()
{
    return m_Nodes;
}

void SyncDCTopologyOCS::GenerateCircuitMatrix()
{
    // now we use round-robin, for num servers we have num matrix
    // and within each matrix, one server is full connected to
    // other servers
    uint32_t dim = m_numNodes;
    for (uint32_t i = 0; i < m_numNodes; i++)
    {
        // num x num matrix
        CircuitMatrix matrix(dim, std::vector<bool>(dim, false));
        for (uint32_t j = 0; j < dim; j++)
        {
            for (uint32_t k = 0; k < dim; k++)
            {
                if (j == i || k == i)
                {
                    matrix[j][k] = true;
                }
            }
        }

        m_circuitMatrix.push_back(matrix);
    }
}

std::vector<CircuitMatrix>
SyncDCTopologyOCS::GetCircuitMatrix()
{
    return m_circuitMatrix;
}

ns3::Time
SyncDCTopologyOCS::GetReConfTime()
{
    return m_reConfTime;
}

ns3::Time
SyncDCTopologyOCS::GetSyncErrorTime()
{
    return m_syncErrorTime;
}

std::string
SyncDCTopologyOCS::GetLinkBandwidth()
{
    return m_linkBandwidth;
}

std::string
SyncDCTopologyOCS::GetLinkDelay()
{
    return m_linkDelay;
}

ns3::Ipv4Address
SyncDCTopologyOCS::GetNodeAddr(uint32_t nodeId)
{
    NS_LOG_FUNCTION(this << nodeId);
    return GetNodeAddr(nodeId, 0);
}

ns3::Ipv4Address
SyncDCTopologyOCS::GetNodeAddr(uint32_t nodeId, uint32_t targetId)
{
    NS_LOG_FUNCTION(this << nodeId << targetId);
    if (nodeId >= m_numNodes || targetId >= m_numNodes)
    {
        NS_LOG_ERROR("Node ID " << nodeId << " or " << targetId << " is out of range");
        return ns3::Ipv4Address();
    }

    return m_MapId2Addr[nodeId][targetId];
}

std::map<ns3::Ipv4Address, uint32_t>
SyncDCTopologyOCS::GetMapAddr2Id()
{
    return m_MapAddr2Id;
}

uint64_t
SyncDCTopologyOCS::GetMsgSize()
{
    return m_config.msgSize;
}
