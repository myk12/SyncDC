#include "paxos-frame.h"

// Define log
NS_LOG_COMPONENT_DEFINE("PaxosFrame");

//********************************************************
//          RequestFrame
//********************************************************
ns3::TypeId RequestFrame::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("RequestFrame")
        .SetParent<ns3::Header>()
        .SetGroupName("Paxos")
        .AddConstructor<RequestFrame>();
    
    return tid;
}

ns3::TypeId RequestFrame::GetInstanceTypeId(void) const {
    return GetTypeId();
}

RequestFrame::RequestFrame() : m_timestamp(0), m_value(0) {}
RequestFrame::RequestFrame(ns3::Time timestamp, uint32_t value)
    : m_timestamp(timestamp), m_value(value) {}
RequestFrame::~RequestFrame() {
    // Destructor logic if needed
}

void RequestFrame::Print(std::ostream &os) const {
    os << "RequestFrame: Timestamp=" << m_timestamp
       << ", Value=" << m_value;
}

uint32_t RequestFrame::GetSerializedSize(void) const {
    return 8 + 4; // Timestamp (8 bytes) + Value (4 bytes)
}

void RequestFrame::Serialize(ns3::Buffer::Iterator start) const {
    // Convert ns3::Time to uint64_t for serialization
    uint64_t timestamp = m_timestamp.GetNanoSeconds();
    start.WriteU64(timestamp);
    start.WriteU32(m_value);
}

uint32_t RequestFrame::Deserialize(ns3::Buffer::Iterator start) {
    // Convert uint64_t back to ns3::Time
    uint64_t timestamp = start.ReadU64();
    // Convert to 1233ns format
    m_timestamp = ns3::Time(std::to_string(timestamp) + "ns");
    m_value = start.ReadU32();
    return GetSerializedSize();
}

ns3::Time RequestFrame::GetTimestamp() const { return m_timestamp; }
uint32_t RequestFrame::GetValue() const { return m_value; }

void RequestFrame::SetTimestamp(ns3::Time timestamp) { m_timestamp = timestamp; }
void RequestFrame::SetValue(uint32_t value) { m_value = value; }

//********************************************************
//              PaxosFrame
//********************************************************

ns3::TypeId PaxosFrame::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("PaxosFrame")
        .SetParent<ns3::Header>()
        .SetGroupName("Paxos")
        .AddConstructor<PaxosFrame>();
        return tid;
}

ns3::TypeId PaxosFrame::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void PaxosFrame::Print(std::ostream &os) const {
    os << "PaxosFrame: MessageType=" << m_messageType
       << ", ProposerId=" << m_proposerId
       << ", ProposalId=" << m_proposalId
       << ", Value=" << m_value
       << ", ProposeTime=" << m_proposeTime
       << ", AcceptorId=" << m_acceptorId
       << ", AcceptTime=" << m_acceptTime
       << ", DecisionTime=" << m_decisionTime;
       }

uint32_t PaxosFrame::GetSerializedSize(void) const {
    // The ns3::Time will be 8 bytes
    return sizeof(m_messageType)
       + sizeof(m_proposerId)
       + sizeof(m_proposalId)
       + sizeof(m_value)
       + sizeof(uint64_t) // ns3::Time m_proposeTime
       + sizeof(m_acceptorId)
       + sizeof(uint64_t) // ns3::Time m_acceptTime
       + sizeof(uint64_t); // ns3::Time m_decisionTime;
}

void PaxosFrame::Serialize(ns3::Buffer::Iterator start) const {
    start.WriteU32(m_messageType);
    start.WriteU32(m_proposerId);
    start.WriteU64(m_proposalId);
    start.WriteU32(m_value);
    // Convert ns3::Time to uint64_t for serialization
    uint64_t proposeTime = m_proposeTime.GetNanoSeconds();
    start.WriteU64(proposeTime);
    start.WriteU32(m_acceptorId);
    // Convert ns3::Time to uint64_t for serialization
    uint64_t acceptTime = m_acceptTime.GetNanoSeconds();
    start.WriteU64(acceptTime);
    // Convert ns3::Time to uint64_t for serialization
    uint64_t decisionTime = m_decisionTime.GetNanoSeconds();
    start.WriteU64(decisionTime);
}

uint32_t PaxosFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_messageType = start.ReadU32();
    m_proposerId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_value = start.ReadU32();
    // Convert uint64_t back to ns3::Time
    uint64_t proposeTime = start.ReadU64();
    // Convert to 1233ns format
    m_proposeTime = ns3::Time(std::to_string(proposeTime) + "ns");
    m_acceptorId = start.ReadU32();
    // Convert uint64_t back to ns3::Time
    uint64_t acceptTime = start.ReadU64();
    // Convert to 1233ns format
    m_acceptTime = ns3::Time(std::to_string(acceptTime) + "ns");
    uint64_t decisionTime = start.ReadU64();
    // Convert to 1233ns format
    m_decisionTime = ns3::Time(std::to_string(decisionTime) + "ns");
    return GetSerializedSize();
}

PaxosFrame::PaxosFrame() : m_messageType(0), m_proposerId(0), m_proposalId(0), m_value(0), m_proposeTime(0), m_acceptorId(0), m_acceptTime(0), m_decisionTime(0) {}
PaxosFrame::~PaxosFrame() {
    // Destructor logic if needed
}

uint32_t PaxosFrame::GetMessageType() const { return m_messageType; }
void PaxosFrame::SetMessageType(uint32_t messageType) { m_messageType = messageType; }
uint32_t PaxosFrame::GetProposerId() const { return m_proposerId; }
void PaxosFrame::SetProposerId(uint32_t proposerId) { m_proposerId = proposerId; }
uint64_t PaxosFrame::GetProposalId() const { return m_proposalId; }
void PaxosFrame::SetProposalId(uint64_t proposalId) { m_proposalId = proposalId; }
uint32_t PaxosFrame::GetValue() const { return m_value; }
void PaxosFrame::SetValue(uint32_t value) { m_value = value; }

ns3::Time PaxosFrame::GetProposeTime() const { return m_proposeTime; }
void PaxosFrame::SetProposeTime(ns3::Time proposeTime) { m_proposeTime = proposeTime; }

uint32_t PaxosFrame::GetAcceptorId() const { return m_acceptorId; }
void PaxosFrame::SetAcceptorId(uint32_t acceptorId) { m_acceptorId = acceptorId; }

ns3::Time PaxosFrame::GetAcceptTime() const { return m_acceptTime; }
void PaxosFrame::SetAcceptTime(ns3::Time acceptTime) { m_acceptTime = acceptTime; }

ns3::Time PaxosFrame::GetDecisionTime() const { return m_decisionTime; }
void PaxosFrame::SetDecisionTime(ns3::Time decisionTime) { m_decisionTime = decisionTime; } 

bool PaxosFrame::IsProposal() const { return m_messageType == PROPOSAL; }
bool PaxosFrame::IsAccept() const { return m_messageType == ACCEPT; }
bool PaxosFrame::IsDecision() const { return m_messageType == DECISION; }
