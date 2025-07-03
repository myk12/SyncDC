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
    : m_target(ns3::Ipv4Address::GetAny(), 0)
{
}

AppOCSAllReduce::AppOCSAllReduce(uint32_t selfId,
                                 ns3::Ipv4Address targetAddr,
                                 ns3::Ipv4Address bindAddr,
                                 ns3::Ipv4Address listenAddr,
                                 uint64_t constMsgSize,
                                 std::string logDir)
    : m_selfId(selfId),
      m_targetAddr(targetAddr),
      m_bindAddr(bindAddr),
      m_listenAddr(listenAddr),
      m_constMsgSize(constMsgSize),
      m_logDir(logDir),
      m_target(m_targetAddr, OCS_ALL_REDUCE_PORT)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("- Start AppOCSAllReduce on server " << m_selfId);
    NS_LOG_INFO("- Target address: " << m_targetAddr << " port " << OCS_ALL_REDUCE_PORT);
    NS_LOG_INFO("- Bind address: " << m_bindAddr << " port " << OCS_ALL_REDUCE_PORT);
    NS_LOG_INFO("- Listen address: " << m_listenAddr << " port " << OCS_ALL_REDUCE_PORT);
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
    m_logFileName = std::string(m_logDir.c_str()) + "/node-" + std::to_string(m_selfId) + ".log";
    m_logFile.open(m_logFileName, std::ios::out | std::ios::app);
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
    ns3::InetSocketAddress local = ns3::InetSocketAddress(m_listenAddr, OCS_ALL_REDUCE_PORT);
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
    ns3::Ptr<ns3::Packet> packet;
    while ((packet = socket->Recv()))
    {
        // Get the timestamp tag
        ns3::TimestampTag timestampTag;
        packet->RemovePacketTag(timestampTag);
        ns3::Time sendTs = timestampTag.GetTimestamp();
        ns3::Time recvTs = ns3::Simulator::Now();
        ns3::Time delay = recvTs - sendTs;

        uint32_t pktSize = packet->GetSize();

        m_logFile << delay.GetNanoSeconds() << std::endl;

        // Receive data
        m_recvBytes += packet->GetSize();

        if (m_recvBytes >= m_constMsgSize)
        {
            NS_LOG_INFO("Server " << m_selfId << " finished receiving data from " << socket);
            socket->Close();
            break;
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
    NS_LOG_INFO("Server " << m_selfId << " creating socket to send data to " << m_targetAddr << " port " << OCS_ALL_REDUCE_PORT);
    m_sendSocket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
    if (m_sendSocket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }
    m_sendSocket->Bind(ns3::InetSocketAddress(m_bindAddr, OCS_ALL_REDUCE_PORT));

    // Send data to target server
    m_target = ns3::InetSocketAddress(m_targetAddr, OCS_ALL_REDUCE_PORT);
    m_sendSocket->Connect(m_target);

    SendData();
}

void AppOCSAllReduce::SendData()
{
    NS_LOG_FUNCTION(this);
    // Create a packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(OCS_ALL_REDUCE_PACKET_SIZE / 2);
    ns3::TimestampTag timestampTag(ns3::Simulator::Now());
    packet->AddPacketTag(timestampTag);

    // Send data
    //NS_LOG_INFO("Server " << m_selfId << " sending data to " << m_targetAddr << " port " << OCS_ALL_REDUCE_PORT);

    m_sendSocket->SendTo(packet, 0, m_target);
    m_sendBytes += packet->GetSize();
    if (m_sendBytes >= m_constMsgSize)
    {
        NS_LOG_INFO("Server " << m_selfId << " finished sending data to " << m_targetAddr << " port " << OCS_ALL_REDUCE_PORT);
        m_sendSocket->Close();
    }
    else
    {
        ns3::Simulator::Schedule(ns3::MicroSeconds(1), &AppOCSAllReduce::SendData, this);
    }
}

