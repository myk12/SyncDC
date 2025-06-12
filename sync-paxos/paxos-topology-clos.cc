#include "paxos-topology-clos.h"

NS_LOG_COMPONENT_DEFINE("PaxosTopologyClos");

PaxosTopologyClos::PaxosTopologyClos(uint32_t numSpines,
                                     uint32_t numLeaves,
                                     uint32_t numHostsPerLeaf,
                                     std::string bandwidthLeaf2Spine,
                                     std::string delayLeaf2Spine,
                                     std::string bandwidthHost2Leaf,
                                     std::string delayHost2Leaf,
                                     PaxosConfig paxosConfig) {



    NS_LOG_INFO("Creating Clos topology with " << numSpines << " spines, " << numLeaves << " leaves, " << numHostsPerLeaf << " hosts per leaf, " << bandwidthLeaf2Spine << " bandwidth leaf to spine, " << delayLeaf2Spine << " delay leaf to spine, " << bandwidthHost2Leaf << " bandwidth host to leaf, " << delayHost2Leaf << " delay host to leaf");

    m_paxosConfig = paxosConfig;

    // Create spine nodes
    m_spineNodes.Create(numSpines);

    // Create leaf nodes
    m_leafNodes.Create(numLeaves);

    // Create host nodes
    for (uint32_t i = 0; i < numLeaves; i++) {
        ns3::NodeContainer hostNodes;
        hostNodes.Create(numHostsPerLeaf);
        m_hostNodes.push_back(hostNodes);
    }

    // Install Network Stacks
    NS_LOG_INFO("Installing network stacks");
    ns3::InternetStackHelper internet;
    internet.Install(m_spineNodes);
    internet.Install(m_leafNodes);
    for (auto it = m_hostNodes.begin(); it != m_hostNodes.end(); it++) {
        internet.Install(*it);
    }

    // Create point-to-point links between spine and leaf
    ns3::PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", ns3::StringValue(bandwidthLeaf2Spine));
    p2p.SetChannelAttribute("Delay", ns3::StringValue(delayLeaf2Spine));

    for (uint32_t i = 0; i < numSpines; i++) {
        std::vector<ns3::NetDeviceContainer> spineLeafLinksMatrixRow;
        std::vector<ns3::Ipv4InterfaceContainer> spineLeafInterfaceMatrixRow;
        for (uint32_t j = 0; j < numLeaves; j++) {
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

    for (uint32_t i = 0; i < numLeaves; i++) {
        std::vector<ns3::NetDeviceContainer> leafHostLinksMatrixRow;
        std::vector<ns3::Ipv4InterfaceContainer> leafHostInterfaceMatrixRow;
        for (uint32_t j = 0; j < numHostsPerLeaf; j++) {
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

    // Log the topology visually
    NS_LOG_INFO("Clos topology created");
    NS_LOG_INFO("Spine nodes:");
    for (uint32_t i = 0; i < numSpines; i++) {
        NS_LOG_INFO("   ---- Spine node " << i << ": " << m_spineNodes.Get(i)->GetId());
        // Log the connected leafnode
        for (uint32_t j = 0; j < numLeaves; j++) {
            NS_LOG_INFO("       |--- Leaf node " << j << ": " << m_spineLeafInterfaceMatrix[i][j].GetAddress(1));
            for (uint32_t k = 0; k < numHostsPerLeaf; k++) {
                NS_LOG_INFO("       |       |--- Host node " << k << ": " << m_hostLeafInterfaceMatrix[j][k].GetAddress(1));
            }
        }
    }

    // Populate the Routing table
    ns3::Ipv4GlobalRoutingHelper::PopulateRoutingTables();

}

PaxosTopologyClos::~PaxosTopologyClos() {
}

ns3::Ipv4Address PaxosTopologyClos::GetSpineAddress(uint32_t spineId) {
    return m_spineLeafInterfaceMatrix[spineId][0].GetAddress(1);
}

ns3::Ipv4Address PaxosTopologyClos::GetLeafAddress(uint32_t spineId, uint32_t leafId) {
    return m_spineLeafInterfaceMatrix[spineId][leafId].GetAddress(1);
}

ns3::Ipv4Address PaxosTopologyClos::GetHostAddress(uint32_t leafId, uint32_t hostId) {
    return m_hostLeafInterfaceMatrix[leafId][hostId].GetAddress(1);
}

int32_t
PaxosTopologyClos::InitPaxosServerCluster(std::vector<std::pair<uint32_t, uint32_t>> hostIdList) {
    NS_LOG_INFO("Initializing Paxos servers");
    int32_t ret = 0;

    // Collect all hosts Info
    for (uint32_t i=0; i<hostIdList.size(); i++) {
        NS_LOG_INFO("   ---- Host " << hostIdList[i].first << " on leaf " << hostIdList[i].second);
        u_int32_t leafId = hostIdList[i].first;
        u_int32_t hostId = hostIdList[i].second;

        // Get the Ip address of the host
        ns3::Ipv4Address hostAddress = GetHostAddress(leafId, hostId);

        // Create a node info
        NodeInfo nodeInfo;
        nodeInfo.serverId   = i;
        nodeInfo.address    = hostAddress;
        nodeInfo.serverPort = SERVER_PORT;
        nodeInfo.paxosPort  = PAXOS_PORT;

        m_serverInfoList.push_back(nodeInfo);
    }

    for (int32_t i=0; i<hostIdList.size(); i++) {
        NS_LOG_INFO("   ---- Creating Paxos server " << i << " on host " << m_serverInfoList[i].address << "" );
        ns3::Ptr<ns3::Node> node = m_hostNodes[hostIdList[i].first].Get(hostIdList[i].second);

        // Create PaxosAppServer and Install on this node
        ns3::Ptr<PaxosAppServer> paxosAppServer = ns3::CreateObject<PaxosAppServer>(i, m_serverInfoList);
        paxosAppServer->SetClockSyncError(ns3::Time(m_paxosConfig.clockSyncError));
        paxosAppServer->SetBoundedMessageDelay(ns3::Time(m_paxosConfig.boundedMessageDelay));
        paxosAppServer->SetNodeFailureRate(m_paxosConfig.nodeFailureRate);

        m_paxosAppServerContainer.Add(paxosAppServer);
        node->AddApplication(paxosAppServer);
    }

    return ret;
}

int32_t
PaxosTopologyClos::InitPaxosClientCluster(std::vector<uint32_t> spineIdList) {
    NS_LOG_INFO("Initializing Paxos clients");
    int32_t ret = 0;

    for (uint32_t i=0; i<spineIdList.size(); i++) {
        NS_LOG_INFO("   ---- Creating Paxos client " << i << " on spine " << spineIdList[i] );
        ns3::Ptr<ns3::Node> node = m_spineNodes.Get(spineIdList[i]);

        // Create PaxosAppClient and Install on this node
        ns3::Ptr<PaxosAppClient> paxosAppClient = ns3::CreateObject<PaxosAppClient>(m_serverInfoList);
        m_paxosAppClientContainer.Add(paxosAppClient);
        node->AddApplication(paxosAppClient);
    }

    return ret;
}

void
PaxosTopologyClos::SetPaxosServerAppStartStop(ns3::Time start, ns3::Time end) {
    for (auto it = m_paxosAppServerContainer.Begin(); it != m_paxosAppServerContainer.End(); it++) {
        (*it)->SetStartTime(start);
        (*it)->SetStopTime(end);
    }
}

void
PaxosTopologyClos::SetPaxosClientAppStartStop(ns3::Time start, ns3::Time end) {
    for (auto it = m_paxosAppClientContainer.Begin(); it != m_paxosAppClientContainer.End(); it++) {
        (*it)->SetStartTime(start);
        (*it)->SetStopTime(end);
    }
}
