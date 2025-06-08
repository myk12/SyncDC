#include "paxos-frame.h"

// Proposal Frame

// Define log
NS_LOG_COMPONENT_DEFINE("PaxosFrame");

ns3::TypeId ProposalFrame::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("ProposalFrame")
        .SetParent<ns3::Header>()
        .SetGroupName("Paxos")
        .AddConstructor<ProposalFrame>();
    return tid;
}
ns3::TypeId ProposalFrame::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void ProposalFrame::Print(std::ostream &os) const {
    os << "ProposalFrame: ProposerId=" << m_proposerId
       << ", ProposalId=" << m_proposalId
       << ", Value=" << m_value
       << ", Timestamp=" << m_timestamp;
}

uint32_t ProposalFrame::GetSerializedSize(void) const {
    return 4 + 8 + 4 + 8; // ProposerId (4 bytes) + ProposalId (8 bytes) + Value (4 bytes) + Timestamp (8 bytes)
}

void ProposalFrame::Serialize(ns3::Buffer::Iterator start) const {
    start.WriteU32(m_proposerId);
    start.WriteU64(m_proposalId);
    start.WriteU32(m_value);
    // Convert ns3::Time to uint64_t for serialization
    uint64_t timestamp = m_timestamp.GetNanoSeconds();
    start.WriteU64(timestamp);
}

uint32_t ProposalFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_proposerId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_value = start.ReadU32();
    
    // Convert uint64_t back to ns3::Time
    uint64_t timestamp = start.ReadU64();
    // Convert to 1233ns format
    m_timestamp = ns3::Time(std::to_string(timestamp) + "ns");

    return GetSerializedSize();
}

ProposalFrame::ProposalFrame() : m_proposerId(0), m_proposalId(0), m_value(0), m_timestamp(0) {}

ProposalFrame::ProposalFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp)
    : m_proposerId(proposerId), m_proposalId(proposalId), m_value(value), m_timestamp(timestamp) {}

ProposalFrame::~ProposalFrame() {
    // Destructor logic if needed
}

uint32_t ProposalFrame::GetProposerId() const { return m_proposerId; }
uint64_t ProposalFrame::GetProposalId() const { return m_proposalId; }
uint32_t ProposalFrame::GetValue() const { return m_value; }
ns3::Time ProposalFrame::GetTimestamp() const { return m_timestamp; }


// Accept Frame
ns3::TypeId AcceptFrame::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("AcceptFrame")
        .SetParent<ns3::Header>()
        .SetGroupName("Paxos")
        .AddConstructor<AcceptFrame>();
    return tid;
}

ns3::TypeId AcceptFrame::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void AcceptFrame::Print(std::ostream &os) const {
    os << "AcceptFrame: AcceptorId=" << m_acceptorId
       << ", ProposalId=" << m_proposalId
       << ", NodeId=" << m_nodeId
       << ", Timestamp=" << m_timestamp;
}

uint32_t AcceptFrame::GetSerializedSize(void) const {
    return 4 + 8 + 4 + 8; // AcceptorId (4 bytes) + ProposalId (8 bytes) + NodeId (4 bytes) + Timestamp (8 bytes)
}

void AcceptFrame::Serialize(ns3::Buffer::Iterator start) const {
    start.WriteU32(m_acceptorId);   
    start.WriteU64(m_proposalId);
    start.WriteU32(m_nodeId);
    // Convert ns3::Time to uint64_t for serialization
    uint64_t timestamp = m_timestamp.GetNanoSeconds();
    start.WriteU64(timestamp);
}

uint32_t AcceptFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_acceptorId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_nodeId = start.ReadU32();
    // Convert uint64_t back to ns3::Time
    uint64_t timestamp = start.ReadU64();
    m_timestamp = ns3::Time(std::to_string(timestamp) + "ns");
    return GetSerializedSize();
}

AcceptFrame::AcceptFrame() : m_acceptorId(0), m_proposalId(0), m_nodeId(0), m_timestamp(0) {}

AcceptFrame::AcceptFrame(uint32_t acceptorId, uint64_t proposalId, uint32_t nodeId, ns3::Time timestamp)
    : m_acceptorId(acceptorId), m_proposalId(proposalId), m_nodeId(nodeId), m_timestamp(timestamp) {}

AcceptFrame::~AcceptFrame() {
    // Destructor logic if needed
}

uint32_t AcceptFrame::GetAcceptorId() const { return m_acceptorId; }
uint64_t AcceptFrame::GetProposalId() const { return m_proposalId; }
uint32_t AcceptFrame::GetNodeId() const { return m_nodeId; }
ns3::Time AcceptFrame::GetTimestamp() const { return m_timestamp; }

// Decision Frame

ns3::TypeId DecisionFrame::GetTypeId(void) {
    static ns3::TypeId tid = ns3::TypeId("DecisionFrame")
        .SetParent<ns3::Header>()
        .SetGroupName("Paxos")
        .AddConstructor<DecisionFrame>();
    return tid;
}

ns3::TypeId DecisionFrame::GetInstanceTypeId(void) const {
    return GetTypeId();
}

void DecisionFrame::Print(std::ostream &os) const {
    os << "DecisionFrame: DecisionId=" << m_proposerId
       << ", ProposalId=" << m_proposalId
       << ", Value=" << m_value
       << ", Timestamp=" << m_timestamp;
}

uint32_t DecisionFrame::GetSerializedSize(void) const {
    return 4 + 8 + 4 + 8; // DecisionId (4 bytes) + Value (4 bytes) + Timestamp (8 bytes)
}

void DecisionFrame::Serialize(ns3::Buffer::Iterator start) const {
    start.WriteU32(m_proposerId);
    start.WriteU64(m_proposalId);
    start.WriteU32(m_value);
    uint64_t timestamp = m_timestamp.GetNanoSeconds();
    start.WriteU64(timestamp);
}

uint32_t DecisionFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_proposerId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_value = start.ReadU32();
    // Convert uint64_t back to ns3::Time
    uint64_t timestamp = start.ReadU64();
    m_timestamp = ns3::Time(std::to_string(timestamp) + "ns");
    return GetSerializedSize();
}   

DecisionFrame::DecisionFrame()
    : m_proposerId(0), m_proposalId(0), m_value(0), m_timestamp(0) {}

DecisionFrame::DecisionFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp)
    : m_proposerId(proposerId), m_proposalId(proposalId), m_value(value), m_timestamp(timestamp) {}

DecisionFrame::~DecisionFrame() {
    // Destructor logic if needed
}

uint32_t DecisionFrame::GetProposerId() const { return m_proposerId; }
uint64_t DecisionFrame::GetProposalId() const { return m_proposalId; }  
uint32_t DecisionFrame::GetValue() const { return m_value; }
ns3::Time DecisionFrame::GetTimestamp() const { return m_timestamp; }

