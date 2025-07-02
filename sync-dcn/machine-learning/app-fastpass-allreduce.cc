#include "app-fastpass-allreduce.h"

NS_LOG_COMPONENT_DEFINE("AppFastpassAllreduce");

ns3::TypeId
AppFastpassAllreduce::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("AppFastpassAllreduce")
                                 .SetParent<ns3::Application>()
                                 .AddConstructor<AppFastpassAllreduce>();
    return tid;
}

AppFastpassAllreduce::AppFastpassAllreduce()
    : m_selfIdx(0),
      m_selfNodeId(0),
      m_serversNum(0),
      m_sendAddr(ns3::Ipv4Address::GetAny(), 0),
      m_arbiterAddr(ns3::Ipv4Address::GetAny(), 0)
{
}

AppFastpassAllreduce::AppFastpassAllreduce(uint32_t selfIdx,
                                           std::vector<std::pair<uint32_t, ns3::Ipv4Address>> serverIDAddrs,
                                           uint16_t port,
                                           uint64_t msgSize,
                                           std::string logFile,
                                           ns3::InetSocketAddress arbiterAddr,
                                           uint32_t jobID)
    : m_selfIdx(selfIdx),
      m_serverIDAddrs(serverIDAddrs),
      m_servicePort(port),
      m_selfNodeId(serverIDAddrs[selfIdx].first),
      m_serversNum(serverIDAddrs.size()),
      m_sendAddr(serverIDAddrs[selfIdx].second, port),
      m_msgSize(msgSize),
      m_logPath(logFile),
      m_arbiterAddr(arbiterAddr),
      m_jobID(jobID)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Server " << m_selfNodeId << " has " << m_serversNum << " servers");
    m_recvBytes = 0;
    m_recvRound = 0;
    m_sendRound = 0;

    // Wire to previous and next server
    m_prevIdx = (selfIdx - 1 + m_serversNum) % m_serversNum;
    m_nextIdx = (selfIdx + 1) % m_serversNum;

    // Init log file
    m_delayLogPath = m_logPath + ".delay";
    m_logfile.open(m_logPath, std::ios::out | std::ios::app);
    m_delayLogfile.open(m_delayLogPath, std::ios::out | std::ios::app);
}

AppFastpassAllreduce::~AppFastpassAllreduce()
{
    NS_LOG_FUNCTION(this);
}

void AppFastpassAllreduce::StartApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Server " << m_selfNodeId << " start application");

    // Start Receiver thread
    StartRingRecvThread();

    // Start Sender thread
    StartRingSendThread();
}

void AppFastpassAllreduce::StopApplication()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Server " << m_selfIdx << " stop app.");
}

void AppFastpassAllreduce::StartRingRecvThread()
{
    NS_LOG_FUNCTION(this);
    // Create socket
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), ns3::TcpSocketFactory::GetTypeId());
    if (m_recvSocket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }

    // Bind socket and listen
    ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), m_servicePort);
    m_recvSocket->Bind(local);
    m_recvSocket->Listen();
    NS_LOG_INFO("Server " << m_selfNodeId << " start listening on port " << m_servicePort);

    // Set callback
    // m_recvSocket->ShutdownSend(); // Don't send data to client
    m_recvSocket->SetAcceptCallback(ns3::MakeCallback(&AppFastpassAllreduce::RequestCallback, this),
                                    ns3::MakeCallback(&AppFastpassAllreduce::AcceptCallback, this));
    m_recvSocket->SetCloseCallbacks(ns3::MakeCallback(&AppFastpassAllreduce::PeerCloseCallback, this),
                                    ns3::MakeCallback(&AppFastpassAllreduce::PeerErrorCallback, this));
}

void AppFastpassAllreduce::StartRingSendThread()
{
    NS_LOG_FUNCTION(this);

    m_sendAddr = ns3::InetSocketAddress(m_serverIDAddrs[m_nextIdx].second, m_servicePort);
    NS_LOG_INFO("Server " << m_selfNodeId << " start sending data to " << m_sendAddr);
    // Create a BulkSendApp and start sending data
    StartBulkSendInstance(0);
}

//****************************************************
//             Recv Thread Callbacks
//****************************************************
void AppFastpassAllreduce::RecvDataCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);

    // Receive data
    ns3::Ptr<ns3::Packet> packet;
    while ((packet = socket->Recv()))
    {
        // Process data
        m_recvBytes += packet->GetSize();

        // Get the timestamp tag
        ns3::TimestampTag timestampTag;
        if (packet->FindFirstMatchingByteTag(timestampTag))
        {
            ns3::Time sendTs = timestampTag.GetTimestamp();
            ns3::Time recvTs = ns3::Simulator::Now();
            ns3::Time delay = recvTs - sendTs;
            m_delayLogfile << delay.GetNanoSeconds() << std::endl;
        }

        // NS_LOG_INFO("Server " << m_selfIdx << " receviced a packet");
        //  Check if we have received enough data
        //  for this round
        if (m_recvBytes >= m_msgSize)
        {
            NS_LOG_INFO("Recv round " << m_recvRound);
            NS_LOG_INFO("LogFile " << m_logPath << " Server " << m_selfIdx << " receviced a packet");
            m_logfile << ns3::Simulator::Now().GetNanoSeconds() << ",RECV," << m_recvRound << std::endl;

            // Increment round
            m_recvBytes = 0;

            // If we have finished all rounds then stop app
            if (m_recvRound == m_serversNum - 1)
            {
                NS_LOG_INFO("Recv round " << m_recvRound << " finished");
                m_logfile << ns3::Simulator::Now().GetNanoSeconds() << "," << m_jobID << ",FINISH" << std::endl; // LOG the completion time
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

bool AppFastpassAllreduce::RequestCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket << from);
    // Check if it is our previous server
    ns3::InetSocketAddress socketAddr = ns3::InetSocketAddress::ConvertFrom(from);
    if (socketAddr.GetIpv4() != m_serverIDAddrs[m_prevIdx].second)
    {
        // Accept connection
        NS_FATAL_ERROR("Server " << m_selfNodeId << " get request from wrong server " << socketAddr.GetIpv4());
        return false;
    }

    // Accept connection
    return true;
}

void AppFastpassAllreduce::AcceptCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_FUNCTION(this << socket << from);
    NS_LOG_INFO("Accept connection from " << from);
    m_recvSockets.push_back(socket);
    socket->SetRecvCallback(ns3::MakeCallback(&AppFastpassAllreduce::RecvDataCallback, this));
}

void AppFastpassAllreduce::PeerCloseCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

void AppFastpassAllreduce::PeerErrorCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

//****************************************************
//             Send Thread Callbacks
//****************************************************

void AppFastpassAllreduce::StartBulkSendInstance(uint32_t sendRound)
{
    // send a request to Fastpass Arbiter
    // and get a timestamp
    NS_LOG_INFO("Start BulkSendApp of round :" << sendRound);

    // create a socket to send request to Fastpass Arbiter
    ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket(GetNode(), ns3::TcpSocketFactory::GetTypeId());
    if (socket == nullptr)
    {
        NS_FATAL_ERROR("Create socket failed");
    }

    // connect to Fastpass Arbiter
    socket->Connect(m_arbiterAddr);
    socket->SetConnectCallback(ns3::MakeCallback(&AppFastpassAllreduce::ArbiterConnectSuccessCallback, this),
                               ns3::MakeCallback(&AppFastpassAllreduce::ArbiterConnectErrorCallback, this));
    m_arbiterSockets.push_back(socket);
}

void AppFastpassAllreduce::ArbiterConnectSuccessCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_INFO("Server " << m_selfNodeId << " connect to Fastpass Arbiter success");

    socket->SetRecvCallback(ns3::MakeCallback(&AppFastpassAllreduce::ArbiterRecvDataCallback, this));
    m_arbiterSockets.push_back(socket);

    ns3::Simulator::Schedule(ns3::NanoSeconds(10), &AppFastpassAllreduce::SendRequestToArbiter, this, socket);
}

void AppFastpassAllreduce::SendRequestToArbiter(ns3::Ptr<ns3::Socket> socket)
{
    // send a request to Fastpass Arbiter
    // and get a timestamp
    FastPassHeader header;
    header.SetSrcId(m_serverIDAddrs[m_selfIdx].first);
    header.SetDstId(m_serverIDAddrs[m_nextIdx].first);
    header.SetDataSize(m_msgSize);

    // Create a packet and send to arbiter
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(header);
    int32_t ret = 0;
    ret = socket->Send(packet);
    if (ret < 0)
    {
        NS_FATAL_ERROR("Server " << m_selfNodeId << " send request to Fastpass Arbiter failed");
    }
    NS_LOG_INFO("Server " << m_selfNodeId << " send request to Fastpass Arbiter success");
}

void AppFastpassAllreduce::ArbiterConnectErrorCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    NS_FATAL_ERROR("Server " << m_selfNodeId << " connect to Fastpass Arbiter failed");
}

void AppFastpassAllreduce::ArbiterRecvDataCallback(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    // Receive data
    NS_LOG_INFO("Server " << m_selfNodeId << " receive a response from Fastpass Arbiter" << std::endl);
    ns3::Ptr<ns3::Packet> packet;
    while ((packet = socket->Recv()))
    {
        // Process data
        FastPassHeader header;
        packet->RemoveHeader(header);
        uint32_t success = header.GetSuccess();
        ns3::Time startTime = header.GetStartTime();

        if (success == 1)
        {
            // create a socket to send data to next server
            StartBulkSendRound(m_sendRound++, startTime);
        }
        else
        {
            NS_LOG_WARN("Server " << m_selfNodeId << " get a wrong timestamp from Fastpass Arbiter");
        }
    }
}

void AppFastpassAllreduce::StartBulkSendRound(uint32_t sendRound, ns3::Time startTime)
{
    m_sendBytes[sendRound] = 0;
    // Create a BulkSendApp and start sending data
    ns3::Ptr<ns3::BulkSendApplication> bulkSendApp = ns3::CreateObject<ns3::BulkSendApplication>();
    bulkSendApp->SetAttribute("MaxBytes", ns3::UintegerValue(m_msgSize));
    bulkSendApp->SetAttribute("Remote", ns3::AddressValue(m_sendAddr));
    bulkSendApp->SetAttribute("EnableTimestampTag", ns3::BooleanValue(true));
    m_sendApps[sendRound] = bulkSendApp;

    // Install apps
    GetNode()->AddApplication(bulkSendApp);

    // Start apps
    NS_LOG_INFO("Start BulkSendApp " << sendRound << " at " << ns3::Simulator::Now().GetSeconds() << "s, with Bytes " << m_msgSize);
    ns3::Time now = ns3::Simulator::Now();
    if (m_startTime <= now)
    {
        bulkSendApp->SetStartTime(ns3::Seconds(0.0));
        NS_LOG_INFO("Start BulkSendApp with startTime <= now " << m_startTime << " for round " << sendRound << " at " << ns3::Simulator::Now().GetSeconds() << "s, with Bytes " << m_msgSize);
    }
    else
    {
        bulkSendApp->SetStartTime(m_startTime - now);
        NS_LOG_INFO("Start BulkSendApp with startTime > now " << m_startTime << " for round " << sendRound << " at " << ns3::Simulator::Now().GetSeconds() << "s, with Bytes " << m_msgSize);
    }
    m_logfile << ns3::Simulator::Now().GetNanoSeconds() << ",SEND," << sendRound << std::endl;
}
