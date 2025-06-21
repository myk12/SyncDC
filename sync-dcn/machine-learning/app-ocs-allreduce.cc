#include "app-ocs-allreduce.h"

NS_LOG_COMPONENT_DEFINE ("AppOCSAllReduce");

ns3::TypeId
AppOCSAllReduce::GetTypeId ()
{
    static ns3::TypeId tid = ns3::TypeId ("AppOCSAllReduce")
                              .SetParent<ns3::Application> ()
                              .AddConstructor<AppOCSAllReduce> ();
    return tid;
}

AppOCSAllReduce::AppOCSAllReduce ()
    : m_selfId (0),
      m_serversNum (0)
{
}

AppOCSAllReduce::AppOCSAllReduce (uint32_t selfId, std::vector<ns3::Ipv4Address> serversAddr, std::string linkDelay, std::string linkBandwidth, ns3::Time reConfTime, ns3::Time syncErrorTime)
    : m_selfId (selfId),
      m_serversNum (serversAddr.size()),
      m_serversAddr (serversAddr),
      m_linkDelay (linkDelay),
      m_linkBandwidth (linkBandwidth),
      m_reConfTime (reConfTime),
      m_syncErrorTime (syncErrorTime)
{
    NS_LOG_FUNCTION (this);
    NS_LOG_INFO ("Start AppOCSAllReduce on server " << m_selfId);

    // Initialize map
    for (uint32_t i = 0; i < m_serversNum; i++)
    {
        m_MapAddr2Id[m_serversAddr[i]] = i;
    }

    m_linkBandwidthBps = BandwidthStr2Bps(m_linkBandwidth);
    m_linkDelayNanoSeconds = DelayStr2NanoSeconds(m_linkDelay);

    // Calculate send slot
    m_sendSlot = ns3::NanoSeconds(m_reConfTime.GetNanoSeconds() + 2*m_syncErrorTime.GetNanoSeconds());
    // sendSizePerSlot = m_linkBandwith * (sendSlot - m_linkDelay)
    double sendTime = (m_sendSlot.GetNanoSeconds() - m_linkDelayNanoSeconds) / 1000000000;
    m_sendSizePerSlot = m_linkBandwidthBps * sendTime;
}

AppOCSAllReduce::~AppOCSAllReduce ()
{
    NS_LOG_FUNCTION (this);
}

void
AppOCSAllReduce::StartApplication ()
{
    NS_LOG_FUNCTION (this);
    NS_LOG_INFO ("Start AppOCSAllReduce on server " << m_selfId);

    // Start Receiver thread
    StartOCSRecvThread ();

    // Start Sender thread
    StartOCSSendThread ();
}

void
AppOCSAllReduce::StopApplication ()
{
    NS_LOG_FUNCTION (this);
}

//********************************************************************************
//                  Receiver thread
//********************************************************************************
void
AppOCSAllReduce::StartOCSRecvThread ()
{
    NS_LOG_FUNCTION (this);
    m_recvSocket = ns3::Socket::CreateSocket(GetNode (), ns3::TcpSocketFactory::GetTypeId ());
    if (m_recvSocket == nullptr)
    {
        NS_FATAL_ERROR ("Create socket failed");
    }

    // Bind socket and listen
    ns3::InetSocketAddress local = ns3::InetSocketAddress (ns3::Ipv4Address::GetAny (), OCS_ALL_REDUCE_PORT);
    m_recvSocket->Bind (local);
    m_recvSocket->Listen ();
    NS_LOG_INFO ("Server " << m_selfId << " listening on " << m_serversAddr[m_selfId] << ":" << OCS_ALL_REDUCE_PORT);

    // Set callback
    m_recvSocket->ShutdownSend (); // Don't send data to client
    m_recvSocket->SetAcceptCallback (ns3::MakeCallback (&AppOCSAllReduce::RequestCallback, this),
                                     ns3::MakeCallback (&AppOCSAllReduce::AcceptCallback, this));
    m_recvSocket->SetCloseCallbacks (ns3::MakeCallback (&AppOCSAllReduce::PeerCloseCallback, this),
                                     ns3::MakeCallback (&AppOCSAllReduce::PeerErrorCallback, this));
}

void
AppOCSAllReduce::RecvDataCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);

    // get id
    uint32_t id;
    if (m_MapSocket2Id.find (socket) == m_MapSocket2Id.end ())
    {
        NS_FATAL_ERROR ("Unknown sender socket: " << socket << std::endl);
    }

    ns3::Ptr<ns3::Packet> packet;
    while ((packet = socket->Recv ()))
    {
        // 
        m_recvBytes[id] += packet->GetSize ();
        if (m_recvBytes[id] >= OCS_ALL_REDUCE_DATA_SIZE)
        {
            NS_LOG_INFO ("Server " << m_selfId << " received " << m_recvBytes[id] << " bytes from server " << id);
            m_recvOKNum++;
            if (m_recvOKNum == m_serversNum - 1)
            {
                NS_LOG_INFO ("Server " << m_selfId << " received all data from all servers");
            }

            return;
        }   
    }
}

bool
AppOCSAllReduce::RequestCallback (ns3::Ptr<ns3::Socket> socket, const ns3::Address& from)
{
    NS_LOG_FUNCTION (this << socket << from);
    return true;
}

void
AppOCSAllReduce::AcceptCallback (ns3::Ptr<ns3::Socket> socket, const ns3::Address& from)
{
    NS_LOG_FUNCTION (this << socket << from);
    ns3::Ipv4Address addr = ns3::InetSocketAddress::ConvertFrom (from).GetIpv4 ();
    uint32_t id = m_MapAddr2Id[addr];
    if (id < 0 || id >= m_serversNum)
    {
        NS_FATAL_ERROR ("Unknown sender address: " << addr << std::endl);
    }

    socket->SetRecvCallback(ns3::MakeCallback (&AppOCSAllReduce::RecvDataCallback, this));
    m_MapSocket2Id[socket] = id;
}

void
AppOCSAllReduce::PeerCloseCallback (ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
}

void
AppOCSAllReduce::PeerErrorCallback (ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
}

//********************************************************************************
//                  Sender thread
//********************************************************************************
void
AppOCSAllReduce::StartOCSSendThread ()
{
    NS_LOG_FUNCTION (this);

    // Calculate send time
    ns3::Time firstRound = ns3::NanoSeconds(m_selfId * m_sendSlot.GetNanoSeconds());

    ns3::Simulator::Schedule(firstRound, &AppOCSAllReduce::SendToAllServers, this);
}

void
AppOCSAllReduce::SendToAllServers ()
{
    NS_LOG_FUNCTION (this);
    // Create Bulk Send Apps
    for (uint32_t i = 0; i < m_serversNum; i++)
    {
        if (i == m_selfId)
        {
            continue;
        }

        // Create a BulkSendApp and start sending data
        ns3::Ptr<ns3::Application> app = ns3::CreateObject<ns3::BulkSendApplication> ();
        app->SetAttribute ("Remote", ns3::AddressValue(ns3::InetSocketAddress (m_serversAddr[i], OCS_ALL_REDUCE_PORT)));
        app->SetAttribute ("MaxBytes", ns3::UintegerValue (m_sendSizePerSlot));
        GetNode()->AddApplication (app);

        m_sendApps[i] = app;
        app->SetStartTime(ns3::Seconds(0.0)); // Start now
    }

    // Schedule next send
    ns3::Simulator::Schedule(m_sendSlot, &AppOCSAllReduce::SendToAllServers, this);
}
