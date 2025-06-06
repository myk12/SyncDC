#include "paxos-frame.h"

// Proposal Frame


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
    start.WriteU64(m_timestamp);
}

uint32_t ProposalFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_proposerId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_value = start.ReadU32();
    m_timestamp = start.ReadU64();
    return GetSerializedSize();
}

ProposalFrame::ProposalFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp)
    : m_proposerId(proposerId), m_proposalId(proposalId), m_value(value), m_timestamp(timestamp) {}

uint32_t ProposalFrame::GetProposerId() const { return m_proposerId; }
uint64_t ProposalFrame::GetProposalId() const { return m_proposalId; }
uint32_t ProposalFrame::GetValue() const { return m_value; }
uint64_t ProposalFrame::GetTimestamp() const { return m_timestamp; }


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
    start.WriteU64(m_timestamp);
}

uint32_t AcceptFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_acceptorId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_nodeId = start.ReadU32();
    m_timestamp = start.ReadU64();
    return GetSerializedSize();
}

AcceptFrame::AcceptFrame(uint32_t acceptorId, uint64_t proposalId, uint32_t nodeId, ns3::Time timestamp)
    : m_acceptorId(acceptorId), m_proposalId(proposalId), m_nodeId(nodeId), m_timestamp(timestamp) {}

uint32_t AcceptFrame::GetAcceptorId() const { return m_acceptorId; }
uint64_t AcceptFrame::GetProposalId() const { return m_proposalId; }
uint32_t AcceptFrame::GetNodeId() const { return m_nodeId; }
uint64_t AcceptFrame::GetTimestamp() const { return m_timestamp; }

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
    os << "DecisionFrame: DecisionId=" << m_decisionId
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
    start.WriteU64(m_timestamp);
}

uint32_t DecisionFrame::Deserialize(ns3::Buffer::Iterator start) {
    m_proposerId = start.ReadU32();
    m_proposalId = start.ReadU64();
    m_value = start.ReadU32();
    m_timestamp = start.ReadU64();
    return GetSerializedSize();
}   

DecisionFrame::DecisionFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp)
    : m_proposerId(proposerId), m_proposalId(proposalId), m_value(value), m_timestamp(timestamp) {}

uint32_t DecisionFrame::GetProposerId() const { return m_proposerId; }
uint64_t DecisionFrame::GetProposalId() const { return m_proposalId; }  
uint32_t DecisionFrame::GetValue() const { return m_value; }
ns3::Time DecisionFrame::GetTimestamp() const { return m_timestamp; }

