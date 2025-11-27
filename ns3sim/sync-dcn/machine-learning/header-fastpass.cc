#include "header-fastpass.h"

NS_LOG_COMPONENT_DEFINE("FastPassHeader");

ns3::TypeId
FastPassHeader::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("FastPassHeader")
                                    .SetParent<ns3::Header>()
                                    .SetGroupName("FastPass")
                                    .AddConstructor<FastPassHeader>();
        return tid;
}

ns3::TypeId
FastPassHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

FastPassHeader::FastPassHeader()
    : m_srcId(0),
      m_dstId(0),
      m_dataSize(0),
      m_success(0),
      m_startTime(ns3::Seconds(0))
{
}

FastPassHeader::~FastPassHeader()
{
}

void
FastPassHeader::Serialize(ns3::Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Serialize FastPassHeader: srcId=" << m_srcId
                 << ", dstId=" << m_dstId
                 << ", dataSize=" << m_dataSize
                 << ", success=" << m_success);
    start.WriteHtonU32(m_srcId);
    start.WriteHtonU32(m_dstId);
    start.WriteHtonU64(m_dataSize);
    start.WriteHtonU32(m_success);
    uint64_t time = m_startTime.GetNanoSeconds();
    start.WriteHtonU64(time);
}

uint32_t
FastPassHeader::Deserialize(ns3::Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("Deserialize FastPassHeader");
    m_srcId = start.ReadNtohU32();
    m_dstId = start.ReadNtohU32();
    m_dataSize = start.ReadNtohU64();
    m_success = start.ReadNtohU32();
    uint64_t time = start.ReadNtohU64();
    m_startTime = ns3::NanoSeconds(time);
    NS_LOG_DEBUG("Deserialized FastPassHeader: srcId=" << m_srcId
                 << ", dstId=" << m_dstId
                 << ", dataSize=" << m_dataSize
                 << ", success=" << m_success
                 << ", startTime=" << m_startTime.GetNanoSeconds() << " ns");
    return GetSerializedSize();
}

uint32_t
FastPassHeader::GetSerializedSize() const
{
    return sizeof(m_srcId) 
            + sizeof(m_dstId) 
            + sizeof(m_dataSize) 
            + sizeof(m_success) 
            + sizeof(uint64_t); // m_startTime in ns
}

void
FastPassHeader::Print(std::ostream& os) const
{
    os << "FastPassHeader: srcId=" << m_srcId
       << ", dstId=" << m_dstId
       << ", dataSize=" << m_dataSize
       << ", success=" << m_success
       << ", startTime=" << m_startTime.GetNanoSeconds() << " ns";
}

void
FastPassHeader::SetSrcId(uint32_t srcId)
{
    m_srcId = srcId;
}

uint32_t
FastPassHeader::GetSrcId() const
{
    return m_srcId;
}

void
FastPassHeader::SetDstId(uint32_t dstId)
{
    m_dstId = dstId;
}

uint32_t
FastPassHeader::GetDstId() const
{
    return m_dstId;
}

void
FastPassHeader::SetDataSize(uint32_t dataSize)
{
    m_dataSize = dataSize;
}

uint32_t
FastPassHeader::GetDataSize() const
{
    return m_dataSize;
}

void
FastPassHeader::SetSuccess(uint32_t success)
{
    m_success = success;
}

uint32_t
FastPassHeader::GetSuccess() const
{
    return m_success;
}

void
FastPassHeader::SetStartTime(ns3::Time startTime)
{
    m_startTime = startTime;
}

ns3::Time
FastPassHeader::GetStartTime() const
{
    return m_startTime;
}
NS_OBJECT_ENSURE_REGISTERED(FastPassHeader);
