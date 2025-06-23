#include "app-ring-allreduce.h"

NS_LOG_COMPONENT_DEFINE("AppRingAllReduce");

ns3::TypeId
AppRingAllReduce::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("AppRingAllReduce")
                                 .SetParent<ns3::Application>()
                                 .AddConstructor<AppRingAllReduce>();
    return tid;
}

AppRingAllReduce::AppRingAllReduce()
    : m_selfId(0),
      m_prevId(0),
      m_nextId(0),
      m_serversNum(0),
      m_sendAddr(ns3::Ipv4Address::GetAny(), RING_ALL_REDUCE_PORT)
{
}

AppRingAllReduce::AppRingAllReduce(uint32_t selfId, std::vector<ns3::Ipv4Address> serversAddr)
    : m_selfId(selfId),
      m_serversAddr(serversAddr),
      m_serversNum(serversAddr.size()),
      m_prevId((selfId - 1 + serversAddr.size()) % serversAddr.size()),
      m_nextId((selfId + 1) % serversAddr.size()),
      m_sendAddr(m_serversAddr[selfId], RING_ALL_REDUCE_PORT)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Start AppRingAllReduce on server " << m_selfId << " with prev=" << m_prevId << " next=" << m_nextId);
    m_recvBytes = 0;
    m_recvRound = 0;
    m_sendRound = 0;

    // Init log file
    std::string logFileName = std::string(LOG_DIR) + "node-" + std::to_string(m_selfId) + ".log";
    m_logfile.open(logFileName, std::ios::out | std::ios::app);
}

AppRingAllReduce::~AppRingAllReduce()
{
    NS_LOG_FUNCTION(this);
}

void AppRingAllReduce::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Start AppRingAllReduce on server " << m_selfId);

    // Start Receiver thread
    StartRingRecvThread();

    // Start Sender thread
    StartRingSendThread();
}

void AppRingAllReduce::StopApplication()
{
    NS_LOG_FUNCTION(this);
}

void AppRingAllReduce::StartRingRecvThread()
{
    NS_LOG_FUNCTION(this);
    // Rece data from previous server

    // Create socket
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), ns3::TcpSocketFactory::GetTypeId());
    if (m_recvSocket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }

    // Bind socket and listen
    ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), RING_ALL_REDUCE_PORT);
    m_recvSocket->Bind(local);
    m_recvSocket->Listen();
    NS_LOG_INFO("Server " << m_selfId << " listening on " << m_serversAddr[m_selfId] << ":" << RING_ALL_REDUCE_PORT);

    // Set callback
    m_recvSocket->ShutdownSend(); // Don't send data to client
    m_recvSocket->SetAcceptCallback(ns3::MakeCallback(&AppRingAllReduce::RequestCallback, this),
                                    ns3::MakeCallback(&AppRingAllReduce::AcceptCallback, this));
    m_recvSocket->SetCloseCallbacks(ns3::MakeCallback(&AppRingAllReduce::PeerCloseCallback, this),
                                    ns3::MakeCallback(&AppRingAllReduce::PeerErrorCallback, this));
}

void AppRingAllReduce::StartRingSendThread()
{
    NS_LOG_FUNCTION(this);

    m_sendAddr = ns3::InetSocketAddress(m_serversAddr[m_nextId], RING_ALL_REDUCE_PORT);
    // Create a BulkSendApp and start sending data
    StartBulkSendInstance(0);
}

//****************************************************
//             Recv Thread Callbacks
//****************************************************
void AppRingAllReduce::RecvDataCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    // Receive data
    ns3::Ptr<ns3::Packet> packet;
    while (packet = socket->Recv())
    {
        // Process data
        m_recvBytes += packet->GetSize();

        // Check if we have received enough data
        // for this round
        if (m_recvBytes >= RING_ALL_REDUCE_DATA_SIZE)
        {
            NS_LOG_INFO("Recv round " << m_recvRound);
            m_logfile << ns3::Simulator::Now().GetNanoSeconds() << ",RECV," << m_recvRound << std::endl;

            // Increment round
            m_recvBytes = 0;

            // If we have finished all rounds then stop app
            if (m_recvRound == m_serversNum - 1)
            {
                NS_LOG_INFO("Recv round " << m_recvRound << " finished");
                StopApplication();
                return;
            }
            m_recvRound++;

            // Since we have received enough data
            // we can start a bulk send instance
            StartBulkSendInstance(++m_sendRound);

        }
    }
}

bool AppRingAllReduce::RequestCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket << from);
    // Check if it is our previous server
    ns3::InetSocketAddress socketAddr = ns3::InetSocketAddress::ConvertFrom(from);
    if (socketAddr.GetIpv4() != m_serversAddr[m_prevId])
    {
        // Get serverId from socket address
        uint32_t serverId = 0;
        for (uint32_t i = 0; i < m_serversNum; i++)
        {
            if (socketAddr.GetIpv4() == m_serversAddr[i])
            {
                serverId = i;
                break;
            }
        }

        // Accept connection
        NS_FATAL_ERROR("Server " << m_selfId << "  get request from wrong server " << serverId);
        return false;
    }

    // Accept connection
    return true;
}

void AppRingAllReduce::AcceptCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket << from);
    NS_LOG_INFO("Accept connection from " << from);
    m_recvSockets.push_back(socket);
    socket->SetRecvCallback(ns3::MakeCallback(&AppRingAllReduce::RecvDataCallback, this));
}

void AppRingAllReduce::PeerCloseCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

void AppRingAllReduce::PeerErrorCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

//****************************************************
//             Send Thread Callbacks
//****************************************************

void AppRingAllReduce::StartBulkSendInstance(uint32_t sendRound)
{
    // This send process is very complex, it differs from the 
    // Underline DCN topology
    NS_LOG_FUNCTION(this << sendRound);
    NS_LOG_INFO("Start BulkSendApp of round :" << sendRound);
    m_sendBytes[sendRound] = 0;
    // Create a BulkSendApp and start sending data
    ns3::Ptr<ns3::BulkSendApplication> bulkSendApp = ns3::CreateObject<ns3::BulkSendApplication>();
    bulkSendApp->SetAttribute("MaxBytes", ns3::UintegerValue(RING_ALL_REDUCE_DATA_SIZE));
    bulkSendApp->SetAttribute("Remote", ns3::AddressValue(m_sendAddr));
    m_sendApps[sendRound] = bulkSendApp;

    // Install apps
    GetNode()->AddApplication(bulkSendApp);

    // Start apps
    NS_LOG_INFO("Start BulkSendApp " << sendRound << " at " << ns3::Simulator::Now().GetSeconds() << "s");
    bulkSendApp->SetStartTime(ns3::Seconds(0.0));
    m_logfile << ns3::Simulator::Now().GetNanoSeconds() << ",SEND," << sendRound << std::endl;
}
