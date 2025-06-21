#include "optical-circuit-switching.h"

// Now the NS-3 do not have a module for optical circuit switch.
// So we need to implement it by ourselves.
// This is a simple optical circuit switch that connect
// all nodes with point-to-point links.

// In the future, we can implement a network device model
// that simulates the optical circuit switch.

NS_LOG_COMPONENT_DEFINE("SyncDCTopologyOCS");

SyncDCTopologyOCS::SyncDCTopologyOCS(YAML::Node& config) {
    YAML::Node topoParam = config["network-topology"]["parameters"];
    // Get total servers
    m_reConfTime = ns3::Time(topoParam["reconf_time"].as<std::string>());
    m_syncErrorTime = ns3::Time(topoParam["sync_error"].as<std::string>());
    uint32_t num_servers = topoParam["total_servers"].as<uint32_t>();
    std::string link_bandwidth = topoParam["link_bandwidth"].as<std::string>();
    std::string link_delay = topoParam["link_delay"].as<std::string>();

    // Create Nodes and Connect them with Point-to-Point Links
    m_Nodes.Create(num_servers);

    // Install Internet Stack
    ns3::InternetStackHelper stack;
    stack.Install(m_Nodes);

    // Create Point-to-Point Links
    ns3::PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", ns3::StringValue(link_bandwidth));
    p2p.SetChannelAttribute("Delay", ns3::StringValue(link_delay));

    ns3::Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.0.0.0", "255.0.0.0");

    for (uint32_t i = 0; i < num_servers; i++)
    {
        std::vector<ns3::NetDeviceContainer> linkArray;
        std::vector<ns3::Ipv4InterfaceContainer> ipArray;
        for (uint32_t j = i + 1; j < num_servers; j++)
        {
            // Install P2P links between nodes
            ns3::NetDeviceContainer link = p2p.Install(m_Nodes.Get(i), m_Nodes.Get(j));
            linkArray.push_back(link);

            // Assign IP addresses to the devices
            ns3::Ipv4InterfaceContainer ip = ipv4.Assign(link);
            ipArray.push_back(ip);
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

ns3::Ipv4Address
SyncDCTopologyOCS::GetNodeAddr(uint32_t nodeIndex)
{
    return m_ipMatrix[nodeIndex][0].GetAddress(1);
}

void
SyncDCTopologyOCS::GenerateCircuitMatrix()
{
    // now we use round-robin, for num servers we have num matrix
    // and within each matrix, one server is full connected to 
    // other servers
    uint32_t dim = m_numNodes;
    for (uint32_t i = 0; i < m_numNodes; i++) {
        // num x num matrix
        CircuitMatrix matrix(dim, std::vector<bool>(dim, false));
        for (uint32_t j = 0; j < dim; j++) {
            for (uint32_t k = 0; k < dim; k++) {
                if (j == i || k == i) {
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
