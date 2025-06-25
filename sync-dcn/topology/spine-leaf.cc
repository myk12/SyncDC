#include "spine-leaf.h"

NS_LOG_COMPONENT_DEFINE("SyncDCTopologySpineLeaf");

std::string
Ipv4int2string(uint32_t ip) {
    std::stringstream ss;
    ss << ((ip >> 24) & 0xFF) << "." << ((ip >> 16) & 0xFF) << "." << ((ip >> 8) & 0xFF) << "." << (ip & 0xFF);
    return ss.str();
}

SyncDCTopologySpineLeaf::SyncDCTopologySpineLeaf(SpineLeafTopologyConfig &config)
{
    NS_LOG_INFO("Creating Spine-Leaf topology");
    // Parse config
    m_config = config;
    // Parse topology parameters
    uint32_t numSpines = config.numSpines;
    uint32_t numLeaves = config.numLeaves;
    uint32_t numHostsPerLeaf = config.numHostsPerLeaf;
    std::string bandwidthLeaf2Spine = config.linkBandwidth;
    std::string delayLeaf2Spine = config.linkDelay;
    std::string bandwidthHost2Leaf = config.linkBandwidth;
    std::string delayHost2Leaf = config.linkDelay;

    NS_LOG_INFO("Number of spines: " << numSpines);
    NS_LOG_INFO("Number of leaves: " << numLeaves);
    NS_LOG_INFO("Number of hosts per leaf: " << numHostsPerLeaf);
    // Create spine nodes
    m_spineNodes.Create(numSpines);

    // Create leaf nodes
    m_leafNodes.Create(numLeaves);

    // Create host nodes
    for (uint32_t i = 0; i < numLeaves; i++)
    {
        ns3::NodeContainer hostNodes;
        hostNodes.Create(numHostsPerLeaf);
        m_hostNodes.push_back(hostNodes);

        // Insert nodeId2LeafIdMap
        for (uint32_t j = 0; j < hostNodes.GetN(); j++)
        {
            uint32_t nodeId = hostNodes.Get(j)->GetId();
            m_nodeId2LeafIdMap[nodeId] = i;
        }
    }

    // Install Network Stacks
    NS_LOG_INFO("Installing network stacks");
    ns3::InternetStackHelper internet;
    internet.Install(m_spineNodes);
    internet.Install(m_leafNodes);
    for (auto it = m_hostNodes.begin(); it != m_hostNodes.end(); it++)
    {
        internet.Install(*it);
    }

    // Create point-to-point links between spine and leaf
    ns3::PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", ns3::StringValue(bandwidthLeaf2Spine));
    p2p.SetChannelAttribute("Delay", ns3::StringValue(delayLeaf2Spine));
    p2p.SetQueue("ns3::DropTailQueue", "MaxSize", ns3::QueueSizeValue(ns3::QueueSize("1000p")));

    for (uint32_t i = 0; i < numSpines; i++)
    {
        std::vector<ns3::NetDeviceContainer> spineLeafLinksMatrixRow;
        std::vector<ns3::Ipv4InterfaceContainer> spineLeafInterfaceMatrixRow;
        for (uint32_t j = 0; j < numLeaves; j++)
        {
            // Install P2P link between spine and leaf
            ns3::NetDeviceContainer spineLeafLink = p2p.Install(m_spineNodes.Get(i), m_leafNodes.Get(j));
            spineLeafLinksMatrixRow.push_back(spineLeafLink);

            // Assign IP addresses to the devices
            ns3::Ipv4AddressHelper ipv4;
            std::ostringstream subnet;
            subnet << "10." << i << "." << j << ".0";
            ipv4.SetBase(subnet.str().c_str(), "255.255.255.0");
            ns3::Ipv4InterfaceContainer spineLeafInterface = ipv4.Assign(spineLeafLink);
            spineLeafInterfaceMatrixRow.push_back(spineLeafInterface);
        }
        m_spineLeafLinksMatrix.push_back(spineLeafLinksMatrixRow);
        m_spineLeafInterfaceMatrix.push_back(spineLeafInterfaceMatrixRow);
    }

    // Create point-to-point links between leaf and host
    ns3::PointToPointHelper p2pHost;
    p2pHost.SetDeviceAttribute("DataRate", ns3::StringValue(bandwidthHost2Leaf));
    p2pHost.SetChannelAttribute("Delay", ns3::StringValue(delayHost2Leaf));

    for (uint32_t i = 0; i < numLeaves; i++)
    {
        std::vector<ns3::NetDeviceContainer> leafHostLinksMatrixRow;
        std::vector<ns3::Ipv4InterfaceContainer> leafHostInterfaceMatrixRow;
        for (uint32_t j = 0; j < numHostsPerLeaf; j++)
        {
            // Install P2P link between leaf and host
            ns3::NetDeviceContainer leafHostLink = p2pHost.Install(m_leafNodes.Get(i), m_hostNodes[i].Get(j));
            leafHostLinksMatrixRow.push_back(leafHostLink);

            // Assign IP addresses to the devices
            ns3::Ipv4AddressHelper ipv4;
            std::ostringstream subnet;
            subnet << "20." << i << "." << j << ".0";
            ipv4.SetBase(subnet.str().c_str(), "255.255.255.0");
            ns3::Ipv4InterfaceContainer leafHostInterface = ipv4.Assign(leafHostLink);
            leafHostInterfaceMatrixRow.push_back(leafHostInterface);
        }

        m_hostLeafLinksMatrix.push_back(leafHostLinksMatrixRow);
        m_hostLeafInterfaceMatrix.push_back(leafHostInterfaceMatrixRow);
    }

    // 
    NS_LOG_INFO("Spine-Leaf topology created");

    // Populate routing table
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();
}

SyncDCTopologySpineLeaf::~SyncDCTopologySpineLeaf()
{
    NS_LOG_INFO("Destroying Spine-Leaf topology");
}

ns3::Ipv4Address SyncDCTopologySpineLeaf::GetSpineAddress(uint32_t spineId)
{
    return m_spineLeafInterfaceMatrix[spineId][0].GetAddress(1);
}

ns3::Ipv4Address SyncDCTopologySpineLeaf::GetLeafAddress(uint32_t spineId, uint32_t leafId)
{
    return m_spineLeafInterfaceMatrix[spineId][leafId].GetAddress(1);
}

ns3::Ipv4Address SyncDCTopologySpineLeaf::GetHostAddress(uint32_t leafId, uint32_t hostId)
{
    return m_hostLeafInterfaceMatrix[leafId][hostId].GetAddress(1);
}

uint64_t SyncDCTopologySpineLeaf::GetMsgSize()
{
    return m_config.msgSize;
}

std::string SyncDCTopologySpineLeaf::GetLogDir()
{
    return m_config.logDir;
}

void
SyncDCTopologySpineLeaf::BindServerId2NodeId(uint32_t serverId, uint32_t nodeId)
{
    m_serverId2NodeIdMap[serverId] = nodeId;

    // Update nodeId2LeafIdMap
    m_serverId2LeafIdMap[serverId] = m_nodeId2LeafIdMap[nodeId];
}

uint32_t SyncDCTopologySpineLeaf::GetLeafIdbyServerId(uint32_t serverId)
{
    NS_ASSERT_MSG(m_serverId2NodeIdMap.find(serverId) != m_serverId2NodeIdMap.end(), "Server ID not found");
    return m_serverId2LeafIdMap[serverId];
}

// Your existing methods (GetSpineAddress, GetLeafAddress, GetHostAddress, InitPaxosServerCluster, InitPaxosClientCluster, SetPaxosServerAppStartStop, SetPaxosClientAppStartStop)...

void SyncDCTopologySpineLeaf::ExportTopologyToYaml(const std::string& filename)
{
    NS_LOG_INFO("Exporting topology to YAML file: " << filename);

    try {
        YAML::Node topology;

        // Add metadata
        topology["metadata"]["num_spines"] = m_spineNodes.GetN();
        topology["metadata"]["num_leaves"] = m_leafNodes.GetN();
        topology["metadata"]["num_hosts_per_leaf"] = m_hostNodes.empty() ? 0 : m_hostNodes[0].GetN();

        // Add spine nodes
        for (uint32_t i = 0; i < m_spineNodes.GetN(); ++i) {
            YAML::Node spine;
            spine["id"] = i;
            spine["node_id"] = m_spineNodes.Get(i)->GetId();
            // Collect all interface addresses for this spine
            YAML::Node interfaces;
            for (uint32_t j = 0; j < m_leafNodes.GetN(); ++j) {
                YAML::Node iface;
                iface["leaf_id"] = j;
                iface["ip_address"] = Ipv4int2string(m_spineLeafInterfaceMatrix[i][j].GetAddress(0, 0).Get());
                interfaces.push_back(iface);
            }
            spine["interfaces"] = interfaces;
            topology["spines"].push_back(spine);
        }

        // Add leaf nodes
        for (uint32_t i = 0; i < m_leafNodes.GetN(); ++i) {
            YAML::Node leaf;
            leaf["id"] = i;
            leaf["node_id"] = m_leafNodes.Get(i)->GetId();
            // Collect spine interfaces
            YAML::Node spine_interfaces;
            for (uint32_t j = 0; j < m_spineNodes.GetN(); ++j) {
                YAML::Node iface;
                iface["spine_id"] = j;
                iface["ip_address"] = Ipv4int2string(m_spineLeafInterfaceMatrix[j][i].GetAddress(1, 0).Get());
                spine_interfaces.push_back(iface);
            }
            leaf["spine_interfaces"] = spine_interfaces;
            // Collect host interfaces
            YAML::Node host_interfaces;
            for (uint32_t k = 0; k < m_hostNodes[i].GetN(); ++k) {
                YAML::Node iface;
                iface["host_id"] = k;
                iface["ip_address"] = Ipv4int2string(m_hostLeafInterfaceMatrix[i][k].GetAddress(0, 0).Get());
                host_interfaces.push_back(iface);
            }
            leaf["host_interfaces"] = host_interfaces;
            topology["leaves"].push_back(leaf);
        }

        // Add host nodes
        for (uint32_t i = 0; i < m_leafNodes.GetN(); ++i) {
            for (uint32_t j = 0; j < m_hostNodes[i].GetN(); ++j) {
                YAML::Node host;
                host["id"] = j;
                host["leaf_id"] = i;
                host["node_id"] = m_hostNodes[i].Get(j)->GetId();
                YAML::Node iface;
                iface["ip_address"] = Ipv4int2string(m_hostLeafInterfaceMatrix[i][j].GetAddress(1, 0).Get());
                host["interface"] = iface;
                topology["hosts"].push_back(host);
            }
        }

        // Write to file
        std::ofstream ofs(filename);
        if (!ofs.is_open()) {
            NS_LOG_ERROR("Failed to open file for writing: " << filename);
            return;
        }
        ofs << topology;
        ofs.close();
        NS_LOG_INFO("Successfully exported topology to: " << filename);
    }
    catch (const std::exception& e) {
        NS_LOG_ERROR("Error exporting topology to YAML: " << e.what());
    }
}

std::vector<ns3::NodeContainer> SyncDCTopologySpineLeaf::GetLeafNodeHosts()
{
    return m_hostNodes;
}

std::string SyncDCTopologySpineLeaf::GetLogDir()
{
    return m_config.logDir;
}

std::string
SyncDCTopologySpineLeaf::GetLinkBandwidth()
{
    return m_config.linkBandwidth;
}

uint32_t
SyncDCTopologySpineLeaf::GetNumSpines()
{
    return m_config.numSpines;
}

uint32_t
SyncDCTopologySpineLeaf::GetNumLeaves()
{
    return m_config.numLeaves;
}

uint32_t
SyncDCTopologySpineLeaf::GetNumHostsPerLeaf()
{
    return m_config.numHostsPerLeaf;
}
