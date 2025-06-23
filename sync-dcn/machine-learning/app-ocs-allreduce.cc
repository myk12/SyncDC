#include "app-ocs-allreduce.h"

NS_LOG_COMPONENT_DEFINE("AppOCSAllReduce");

ns3::TypeId
AppOCSAllReduce::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("AppOCSAllReduce")
                                 .SetParent<ns3::Application>()
                                 .AddConstructor<AppOCSAllReduce>();
    return tid;
}

AppOCSAllReduce::AppOCSAllReduce()
    : m_selfId(0),
      m_serversNum(0)
{
}

AppOCSAllReduce::AppOCSAllReduce(uint32_t selfId,
                                 std::vector<ns3::Ipv4Address> serversAddr,
                                 std::string linkDelay, std::string linkBandwidth,
                                 ns3::Time reConfTime, ns3::Time syncErrorTime,
                                 std::map<ns3::Ipv4Address, uint32_t> MapAddr2Id)
    : m_selfId(selfId),
      m_serversAddr(serversAddr),
      m_linkDelay(linkDelay),
      m_linkBandwidth(linkBandwidth),
      m_reConfTime(reConfTime),
      m_syncErrorTime(syncErrorTime)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Start AppOCSAllReduce on server " << m_selfId);
    m_serversNum = m_serversAddr.size();

    // Initialize map
    m_MapAddr2Id = MapAddr2Id;
    m_linkBandwidthBps = BandwidthStr2Bps(m_linkBandwidth);
    m_linkDelayNanoSeconds = DelayStr2NanoSeconds(m_linkDelay);

    // Calculate send slot
    m_sendSlot = ns3::NanoSeconds(m_reConfTime.GetNanoSeconds() + 2 * m_syncErrorTime.GetNanoSeconds());
    m_OCSPeriod = ns3::NanoSeconds((m_sendSlot.GetNanoSeconds() + 2 * m_syncErrorTime.GetNanoSeconds()) * m_serversNum);
    // sendSizePerSlot = m_linkBandwith * (sendSlot - m_linkDelay)
    double sendTime = (m_sendSlot.GetNanoSeconds() - m_linkDelayNanoSeconds);
    m_sendSizePerSlot = m_linkBandwidthBps * sendTime / 1e9; // Convert to bytes

    NS_LOG_INFO("Send slot: " << m_sendSlot << " ns");
    NS_LOG_INFO("Send Time: " << sendTime << " nanoseconds");
    NS_LOG_INFO("Link delay: " << m_linkDelay << " ns");
    NS_LOG_INFO("Link bandwidth: " << m_linkBandwidth << " bps");
    NS_LOG_INFO("Link bandwidth in bytes: " << m_linkBandwidthBps << " Bps");
    NS_LOG_INFO("Link delay in nanoseconds: " << m_linkDelayNanoSeconds << " ns");
    NS_LOG_INFO("Reconfiguration time: " << m_reConfTime << " seconds");
    NS_LOG_INFO("Send size per slot: " << m_sendSizePerSlot << " bytes");
}

AppOCSAllReduce::~AppOCSAllReduce()
{
    NS_LOG_FUNCTION(this);
}

void AppOCSAllReduce::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Start AppOCSAllReduce on server " << m_selfId);

    // Start Receiver thread
    StartOCSRecvThread();

    // Start Sender thread
    StartOCSSendThread();

    // Init Log
    std::string logFilename = std::string(LOG_DIR) + "node-" + std::to_string(m_selfId) + ".log";
    m_logFile.open(logFilename, std::ios::out | std::ios::app);
}

void AppOCSAllReduce::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

//********************************************************************************
//                  Receiver thread
//********************************************************************************
void AppOCSAllReduce::StartOCSRecvThread()
{
    NS_LOG_FUNCTION(this);
    // Use UDP socket to receive data from other servers
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
    if (m_recvSocket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }

    // Bind socket and listen
    ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), OCS_ALL_REDUCE_PORT);
    m_recvSocket->Bind(local);
    m_recvSocket->Listen();
    NS_LOG_INFO("Server " << m_selfId << " listening on " << local.GetIpv4());

    // Set callback
    m_recvSocket->ShutdownSend(); // Don't send data to client
    m_recvSocket->SetRecvCallback(ns3::MakeCallback(&AppOCSAllReduce::RecvDataCallback, this)); 
}

void AppOCSAllReduce::RecvDataCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    //NS_LOG_INFO("Server " << m_selfId << " received data from " << socket);
    // Get remote address
    ns3::Address from;
    socket->RecvFrom(from);
    ns3::Ipv4Address remoteAddr = ns3::InetSocketAddress::ConvertFrom(from).GetIpv4();

    //NS_LOG_INFO("Server " << m_selfId << " received data from server " << remoteAddr << " at " << ns3::Simulator::Now().GetSeconds() << " seconds");
    // Get server ID from address
    uint32_t id = m_MapAddr2Id[remoteAddr];
    //NS_LOG_INFO("Server " << m_selfId << " received data from server " << id << " at " << ns3::Simulator::Now().GetSeconds() << " seconds");

    ns3::Ptr<ns3::Packet> packet;
    while ((packet = socket->Recv()))
    {
        m_recvBytes[id] += packet->GetSize();
        //NS_LOG_INFO ("Server " << m_selfId << " received " << packet->GetSize () << " bytes from server " << id);
        if (m_recvBytes[id] >= OCS_ALL_REDUCE_DATA_SIZE)
        {
            m_logFile <<ns3::Simulator::Now().GetNanoSeconds() << ",RECV,"  << id << std::endl;
            NS_LOG_INFO("Server " << m_selfId << " received " << m_recvBytes[id] << " bytes from server " << id);
            m_recvOKNum++;
            if (m_recvOKNum == m_serversNum - 1)
            {
                NS_LOG_INFO("Server " << m_selfId << " received all data from all servers");
            }

            return;
        }
    }
}

//********************************************************************************
//                  Sender thread
//********************************************************************************
void AppOCSAllReduce::StartOCSSendThread()
{
    NS_LOG_FUNCTION(this);
    // Create Sender socket
    for (uint32_t i = 0; i < m_serversNum; i++)
    {
        if (i == m_selfId)
        {
            // Don't send data to self
            continue;
        }

        // Create a socket for each server
        ns3::Ptr<ns3::Socket> sendSocket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
        if (sendSocket == nullptr)
        {
            NS_FATAL_ERROR("Create socket failed");
        }

        // Bind the socket to a random port
        sendSocket->Bind(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), 0));
        sendSocket->ShutdownRecv(); // Don't receive data from client
        ns3::InetSocketAddress remote = ns3::InetSocketAddress(m_serversAddr[i], OCS_ALL_REDUCE_PORT);
        // Connect to the remote server
        sendSocket->Connect(remote);
        m_sendSockets[i] = sendSocket;
    }

    // Calculate send time
    ns3::Time firstRound = ns3::NanoSeconds(m_selfId * m_sendSlot.GetNanoSeconds());

    m_totalSendBytes = 0;

    ns3::Simulator::Schedule(firstRound, &AppOCSAllReduce::SendToAllServers, this);
}

void AppOCSAllReduce::SendToAllServers()
{
    NS_LOG_FUNCTION(this);
    m_logFile <<ns3::Simulator::Now().GetNanoSeconds() << ",SEND" << std::endl;
    NS_LOG_INFO("Server " << m_selfId << " start sending data to all servers");
    // Create Bulk Send Apps
    ns3::Ptr<ns3::Node> node = GetNode();
    for (uint32_t i = 0; i < m_serversNum; i++)
    {
        if (i == m_selfId)
        {
            // Don't send data to self
            continue;
        }

        SendToServer(i, m_sendSizePerSlot);
    }

    m_totalSendBytes += m_sendSizePerSlot;

    // Schedule next round
    if (m_totalSendBytes < OCS_ALL_REDUCE_DATA_SIZE)
    {
        NS_LOG_INFO("Server " << m_selfId << " scheduled next send at " << ns3::Simulator::Now().GetSeconds() + m_OCSPeriod.GetSeconds() << " seconds");
        ns3::Simulator::Schedule(m_OCSPeriod, &AppOCSAllReduce::SendToAllServers, this);
    }
    else
    {
        NS_LOG_INFO("Server " << m_selfId << " finished sending data to all servers");
    }
}

void 
AppOCSAllReduce::SendToServer(uint32_t targetId, int64_t sendSize)
{
    if (sendSize <= 0)
    {
        NS_LOG_INFO("Finished sending data to server " << targetId);
        return;
    }

    //NS_LOG_INFO("Server " << m_selfId << " sending data to server " << targetId << " with size " << sendSize << " bytes at " << ns3::Simulator::Now().GetSeconds() << " seconds");
    NS_LOG_FUNCTION(this << targetId << sendSize);
    // Check if socket is created
    ns3::Ptr<ns3::Socket> socket;
    if (m_sendSockets.find(targetId) == m_sendSockets.end())
    {
        NS_FATAL_ERROR("Socket not created");
    }

    socket = m_sendSockets[targetId];
    NS_LOG_FUNCTION(this << targetId << socket);

    // Create packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(OCS_ALL_REDUCE_PACKET_SIZE);

    // Send data
    ns3::InetSocketAddress remote = ns3::InetSocketAddress(m_serversAddr[targetId], OCS_ALL_REDUCE_PORT);
    int32_t sent = socket->SendTo(packet, 0, remote);
    if (sent < 0)
    {
        NS_FATAL_ERROR("Error while sending data");
    }

    // Schedule next send
    ns3::Simulator::Schedule(ns3::NanoSeconds(1), &AppOCSAllReduce::SendToServer, this, targetId, sendSize - sent);
}
