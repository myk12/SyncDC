#include "app-all-reduce.h"

NS_LOG_COMPONENT_DEFINE("AppAllReduce");

AppAllReduce::AppAllReduce()
{
    m_targetBytes = TARGET_BYTES;
}

AppAllReduce::~AppAllReduce()
{
}

ns3::TypeId
AppAllReduce::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("AppAllReduce")
                                 .SetParent<ns3::Application>()
                                 .AddConstructor<AppAllReduce>();
    return tid;
}

void AppAllReduce::InitClusterInfo(uint32_t self_id, const std::vector<ns3::Ipv4Address> &serverAddrs)
{
    m_selfId = self_id;
    m_selfAddress = serverAddrs[m_selfId];

    for (uint32_t i = 0; i < serverAddrs.size(); i++)
    {
        m_id2ServerAddr[i] = serverAddrs[i];

        if (i == m_selfId) continue;

        ns3::InetSocketAddress serverAddr = ns3::InetSocketAddress(serverAddrs[i], ALL_REDUCE_PORT);
        // Create Bulk Send APP
        ns3::Ptr<ns3::Application> app = ns3::CreateObject<ns3::BulkSendApplication>();
        app->SetAttribute("Remote", ns3::AddressValue(serverAddr));
        app->SetAttribute("MaxBytes", ns3::UintegerValue(m_targetBytes));
        GetNode()->AddApplication(app);
        m_bulkSendApps.Add(app);
    }    

}

void AppAllReduce::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Server " << m_selfId << " started.");
    int32_t ret = 0;
    // Create listener socket
    m_listenSocket = ns3::Socket::CreateSocket(GetNode(), ns3::TcpSocketFactory::GetTypeId());
    if (m_listenSocket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }

    ret = m_listenSocket->Bind(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), ALL_REDUCE_PORT));
    if (ret != 0)
    {
        NS_FATAL_ERROR("Bind socket failed");
    }

    m_listenSocket->Listen();
    m_listenSocket->ShutdownSend(); // Don't need to send data to client
    NS_LOG_INFO("Server " << m_selfId << " listening on " << m_selfAddress << ":" << ALL_REDUCE_PORT);

    m_listenSocket->SetRecvCallback(ns3::MakeCallback(&AppAllReduce::HandleRead, this));
    m_listenSocket->SetRecvPktInfo(true);
    m_listenSocket->SetAcceptCallback(ns3::MakeCallback(&AppAllReduce::HandleRequest, this),
                                      ns3::MakeCallback(&AppAllReduce::HandleAccept, this));
    m_listenSocket->SetCloseCallbacks(ns3::MakeCallback(&AppAllReduce::HandlePeerClose, this),
                                      ns3::MakeCallback(&AppAllReduce::HandlePeerError, this));

    // set default targetbytes
    m_targetBytes = 32 * 1024;
    NS_LOG_INFO("Target bytes: " << m_targetBytes);

    // clear m_recvBytes
    for (uint32_t i = 0; i < m_id2ServerAddr.size(); i++)
    {
        m_id2RecvBytes[i] = 0;
    }

    // Init log file
    m_logfilename = "server_" + std::to_string(m_selfId) + ".log";

}

void AppAllReduce::StopApplication()
{
    m_listenSocket->Close();
}

bool AppAllReduce::HandleRequest(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket);
    return true;
}

void AppAllReduce::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    // Get the sender address from socket
    ns3::Address from;
    int32_t fromId;
    socket->GetPeerName(from);
    ns3::Ipv4Address fromAddr = ns3::InetSocketAddress::ConvertFrom(from).GetIpv4();

    fromId = GetServerIdByAddress(fromAddr);
    if (fromId < 0 || fromId >= m_id2ServerAddr.size())
    {
        NS_FATAL_ERROR("Unknown sender address");
    }

    ns3::Ptr<ns3::Packet> packet;
    // Recv packet from socket
    while ((packet = socket->Recv()))
    {
        // TODO: handle the received packet
        NS_LOG_INFO("Server " << m_selfId << " received packet from server " << fromId << " at " << m_id2ServerAddr[fromId] << ":" << ALL_REDUCE_PORT);
        m_id2RecvBytes[fromId] += packet->GetSize();
    }

    // Check if we have get enough data
    if (m_id2RecvBytes[fromId] >= m_targetBytes)
    {
        // This server has sent enough data
        // TODO: Record time and move to next round
        NS_LOG_INFO("Server " << m_selfId << " finished receiving "<< m_id2RecvBytes[fromId] << " bytes from server " << fromId << std::endl);
        m_recvOverNum++;

        // save to log 
        std::ofstream log(m_logfilename, std::ios::app);
        log << ns3::Simulator::Now().GetSeconds() << " Recv all " << m_id2RecvBytes[fromId] << " bytes from server " << fromId << std::endl;

        return;
    }

    // Check if we have received all data from all servers
    if (m_recvOverNum == m_id2ServerAddr.size() - 1)
    {
        // All servers have sent data
        // TODO: Record time and move to next round
        NS_LOG_INFO("All servers have sent data");
        return;
    }
}

void AppAllReduce::HandleAccept(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket << from);
    socket->SetRecvCallback(ns3::MakeCallback(&AppAllReduce::HandleRead, this));

    ns3::Ipv4Address fromAddr = ns3::InetSocketAddress::ConvertFrom(from).GetIpv4();
    NS_LOG_INFO("Server " << m_selfId << " accepted connection from " << fromAddr);
    int32_t serverId = GetServerIdByAddress(fromAddr);
    if (serverId == -1)
    {
        NS_LOG_ERROR("Unknown sender address");
        return ;
    }

    // Add socket to m_recvSockets
    NS_LOG_INFO("Server " << m_selfId << " added socket to m_recvSockets");
    m_recvSockets[serverId] = socket;
}

void AppAllReduce::HandlePeerClose(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

void AppAllReduce::HandlePeerError(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

uint32_t 
AppAllReduce::GetServerIdByAddress(const ns3::Ipv4Address& addr)
{
    for (auto it = m_id2ServerAddr.begin(); it != m_id2ServerAddr.end(); it++)
    {
        if (it->second == addr)
        {
            return it->first;
        }
    }
    return -1;
}
