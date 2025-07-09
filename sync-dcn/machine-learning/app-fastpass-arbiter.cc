#include "app-fastpass-arbiter.h"

NS_LOG_COMPONENT_DEFINE("FastPassArbiter");

FastPassArbiter::FastPassArbiter()
{
    // Constructor implementation
}

FastPassArbiter::~FastPassArbiter()
{
    // Destructor implementation
}

ns3::TypeId
FastPassArbiter::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("FastPassArbiter")
                                 .SetParent<ns3::Application>()
                                 .SetGroupName("Applications")
                                 .AddConstructor<FastPassArbiter>();

    return tid;
}

void FastPassArbiter::StartApplication()
{
    // Start application implementation
    InitListener();

    // Init resource pool
    InitResourcePool();
}

void FastPassArbiter::StopApplication()
{
    // Stop application implementation
}

void FastPassArbiter::InitListener()
{
    // Init listener implementation
    m_listenSocket = ns3::Socket::CreateSocket(GetNode(), ns3::TcpSocketFactory::GetTypeId());
    NS_ASSERT_MSG(m_listenSocket, "Failed to create socket.");

    m_listenSocket->Bind(ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), FASTPASS_ARBITER_PORT));
    m_listenSocket->Listen();

    m_listenSocket->SetAcceptCallback(
        ns3::MakeCallback(&FastPassArbiter::HandleAccept, this),
        ns3::MakeCallback(&FastPassArbiter::HandleConnect, this));
    // m_listenSocket->SetRecvCallback(ns3::MakeCallback(&FastPassArbiter::HandleRead, this));
    m_listenSocket->SetCloseCallbacks(ns3::MakeCallback(&FastPassArbiter::HandleNormalClose, this),
                                      ns3::MakeCallback(&FastPassArbiter::HandleErrorClose, this));

    NS_LOG_INFO("FastPassArbiter listening on port " << FASTPASS_ARBITER_PORT);
}

bool FastPassArbiter::HandleAccept(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_INFO("Accepted connection from " << from);
    return true; // Return true to accept the connection
}

void FastPassArbiter::HandleConnect(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from)
{
    NS_LOG_INFO("Connection established with " << from);
    ns3::Ipv4Address ipaddr = ns3::InetSocketAddress::ConvertFrom(from).GetIpv4();
    socket->SetRecvCallback(ns3::MakeCallback(&FastPassArbiter::HandleRead, this));
    m_acceptSockets.push_back(socket);
}

void FastPassArbiter::HandleErrorClose(ns3::Ptr<ns3::Socket> socket)
{
    // Handle error close
    NS_LOG_WARN("Arbiter Connection closed with error");
}

void FastPassArbiter::HandleNormalClose(ns3::Ptr<ns3::Socket> socket)
{
    // Handle normal close
    NS_LOG_INFO("Arbiter Connection closed normally");
}

void FastPassArbiter::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_INFO("Arbiter received a request packet.");
    // Handle read
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    socket->GetSockName(from);
    ns3::Ipv4Address ipaddr = ns3::InetSocketAddress::ConvertFrom(from).GetIpv4();

    while ((packet = socket->Recv()))
    {
        NS_LOG_INFO("Received packet from " << ipaddr << " of size " << packet->GetSize());
        // Process the packet
        FastPassHeader header;
        NS_ASSERT_MSG(packet->PeekHeader(header), "Failed to peek header.");
        ProcessRequest(socket, header);
    }
}

void FastPassArbiter::ProcessRequest(ns3::Ptr<ns3::Socket> socket, FastPassHeader header)
{
    NS_LOG_FUNCTION(this << socket << header);
    NS_LOG_INFO("Processing request");
    uint32_t srcId = header.GetSrcId();
    uint32_t dstId = header.GetDstId();
    uint64_t dataSize = header.GetDataSize();

    uint32_t srcLeafId = m_topology->GetLeafIdbyServerId(srcId);
    uint32_t dstLeafId = m_topology->GetLeafIdbyServerId(dstId);

    NS_LOG_INFO("Processing request from " << srcId << " to " << dstId << " of size " << dataSize << " bytes");

    ns3::Time allocatedTime;
    // If src and dst are in the same leaf
    if (srcLeafId == dstLeafId)
    {
        // Send data to dst
        allocatedTime = AllocateIntraLeafResource(dstLeafId, dataSize);
    }
    else
    {
        // Send data to dst leaf
        allocatedTime = AllocateInterLeafResource(srcLeafId, dstLeafId, dataSize);
    }

    // Response to client
    // Construct response packet
    header.SetSuccess(1);
    header.SetStartTime(allocatedTime);
    ns3::Ptr<ns3::Packet> responsePacket = ns3::Create<ns3::Packet>();
    responsePacket->AddHeader(header);

    // Send response
    SendResponse(socket, responsePacket);
}

void FastPassArbiter::InitResourcePool()
{
    // Init resource pool
    NS_LOG_INFO("Init resource pool");
    uint32_t numSpines = m_topology->GetNumSpines();
    uint32_t numLeafs = m_topology->GetNumLeaves();
    uint32_t hostPerLeaf = m_topology->GetNumHostsPerLeaf();

    // Init resource pool
    for (uint32_t i = 0; i < numLeafs; i++)
    {
        std::vector<ns3::Time> resourcePool(hostPerLeaf / 2, ns3::Simulator::Now());
        m_inLeafResourcePool[i] = resourcePool;
    }

    for (uint32_t i = 0; i < numLeafs; i++)
    {
        std::vector<ns3::Time> resourcePool(numSpines, ns3::Simulator::Now());
        m_crossLeafResourcePool[i] = resourcePool;
    }

    NS_LOG_INFO("Resource pool initialized");
}

ns3::Time
FastPassArbiter::AllocateIntraLeafResource(uint32_t dstLeaf, uint64_t dataSize)
{
    // Get the resource pool of the leaf
    NS_LOG_INFO("Allocating intra-leaf " << dstLeaf << " for " << dataSize << " bytes");
    std::vector<ns3::Time> &resourcePool = m_inLeafResourcePool[dstLeaf];

    // Find the first available resource
    uint32_t leastId = 0;
    ns3::Time leastTime = resourcePool[0];
    for (uint32_t i = 0; i < resourcePool.size(); i++)
    {
        if (resourcePool[i] < leastTime)
        {
            leastId = i;
            leastTime = resourcePool[i];
        }
    }
    NS_LOG_INFO("Find least resource with ID " << leastId << " with time " << leastTime);

    // Allocate the resource
    ns3::Time allocatedTime = leastTime;

    // Update the resource pool
    //resourcePool[leastId] += DataSize2TimeSlot(dataSize);
    resourcePool[leastId] += ns3::MicroSeconds(100);
    NS_LOG_INFO("Allocated intra-leaf resource " << allocatedTime << " for " << dataSize << " bytes");
    return allocatedTime;
}

ns3::Time
FastPassArbiter::AllocateInterLeafResource(uint32_t srcLeafId, uint32_t dstLeaf, uint64_t dataSize)
{
    // Get the resource pool of the leaf
    NS_LOG_INFO("Allocating inter-leaf resource");
    std::vector<ns3::Time> &srcLeftResourcePool = m_crossLeafResourcePool[srcLeafId];
    std::vector<ns3::Time> &dstLeftResourcePool = m_crossLeafResourcePool[dstLeaf];

    uint32_t srcLeastId = 0;
    ns3::Time srcLeastTime = srcLeftResourcePool[0];
    uint32_t dstLeastId = 0;
    ns3::Time dstLeastTime = dstLeftResourcePool[0];

    // Find the first available resource
    for (uint32_t i = 0; i < srcLeftResourcePool.size(); i++)
    {
        if (srcLeftResourcePool[i] < srcLeastTime)
        {
            srcLeastId = i;
            srcLeastTime = srcLeftResourcePool[i];
        }
        if (dstLeftResourcePool[i] < dstLeastTime)
        {
            dstLeastId = i;
            dstLeastTime = dstLeftResourcePool[i];
        }
    }

    // Allocate the resource
    ns3::Time allocatedTime = std::max(srcLeastTime, dstLeastTime);

    // Update the resource pool
    //ns3::Time allocatedSlot = DataSize2TimeSlot(dataSize);
    ns3::Time allocatedSlot = ns3::MicroSeconds(200);
    srcLeftResourcePool[srcLeastId] = allocatedTime + allocatedSlot;
    dstLeftResourcePool[dstLeastId] = allocatedTime + allocatedSlot;

    NS_LOG_UNCOND("Allocated inter-leaf resource " << allocatedTime << " for " << dataSize << " bytes");
    // Response to client
    return allocatedTime;
}

void FastPassArbiter::SendResponse(ns3::Ptr<ns3::Socket> socket, ns3::Ptr<ns3::Packet> responsePacket)
{
    NS_LOG_INFO("Sending response to client");
    socket->Send(responsePacket);
}

ns3::Time
FastPassArbiter::DataSize2TimeSlot(uint64_t dataSize)
{
    // Get the bandwidth of the leaf
    std::string bandwidth = m_topology->GetLinkBandwidth();
    uint64_t bandwidthInBps = BandwidthStr2Bps(bandwidth);
    uint64_t nanoTimeSlot = dataSize * 8 / bandwidthInBps;
    NS_LOG_INFO("Data size " << dataSize << " bytes, bandwidth " << bandwidth << " bps, time slot " << nanoTimeSlot << " ns");

    return ns3::Time(nanoTimeSlot);
}

void FastPassArbiter::SetTopology(std::shared_ptr<SyncDCTopologySpineLeaf> topology)
{
    m_topology = topology;
}

void FastPassArbiter::SetLogDir(std::string logDir)
{
    m_logDir = logDir;
}
