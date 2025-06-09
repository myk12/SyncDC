#ifndef PAXOS_FRAME_HH
#define PAXOS_FRAME_HH

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"

// Request Frame
class RequestFrame : public ns3::Header
{
public:
    RequestFrame();
    RequestFrame(ns3::Time timestamp, uint32_t value);
    ~RequestFrame();
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;

    void Print(std::ostream &os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    // Getters for the request frame fields
    ns3::Time GetTimestamp() const;
    uint32_t GetValue() const;

    // Setters for the request frame fields
    void SetTimestamp(ns3::Time timestamp);
    void SetValue(uint32_t value);

private:
    // Payload fields
    ns3::Time m_timestamp; // Timestamp of the request
    uint32_t  m_value; // Value of the request
};

// Paxos Frame
class PaxosFrame : public ns3::Header
{
public:
    enum MessageType
    {
        PROPOSAL = 100,
        ACCEPT,
        DECISION,
    };

    PaxosFrame();
    ~PaxosFrame();
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;

    void Print(std::ostream &os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    // Getters and Setters for the Paxos frame fields
    uint32_t GetMessageType() const;
    void SetMessageType(uint32_t messageType);
    uint32_t GetProposerId() const;
    void SetProposerId(uint32_t proposerId);
    uint64_t GetProposalId() const;
    void SetProposalId(uint64_t proposalId);
    uint32_t GetValue() const;
    void SetValue(uint32_t value);

    ns3::Time GetProposeTime() const;
    void SetProposeTime(ns3::Time proposeTime);

    uint32_t GetAcceptorId() const;
    void SetAcceptorId(uint32_t acceptorId);

    ns3::Time GetAcceptTime() const;
    void SetAcceptTime(ns3::Time acceptTime);

    ns3::Time GetDecisionTime() const;
    void SetDecisionTime(ns3::Time decisionTime);

    bool IsProposal() const;
    bool IsAccept() const;
    bool IsDecision() const;

private:
    // Message type (4 bytes) - A unique identifier for the message type.
    uint32_t m_messageType; // Type of the message (4 bytes)

    // Propose
    uint32_t m_proposerId; // ID of the proposer
    uint64_t m_proposalId;  // ID of the proposal
    uint32_t m_value;       // Value of the proposal
    ns3::Time m_proposeTime; // Timestamp of the proposal

    // Accept
    uint32_t m_acceptorId;
    ns3::Time m_acceptTime; // Timestamp of the accept

    // Decision
    ns3::Time m_decisionTime; // Timestamp of the decision
};

// This class define the frame format for Paxos messages.
// It is used to encapsulate the messages sent between nodes in the Paxos protocol.
// The general structure of the frame is as follows:    
// 1. Signature (4 bytes) - A unique identifier for the message type.
//    - "PROP" for Proposal messages
//    - "ACPT" for Accept messages
//    - "DECI" for Decision messages

// 2. Payload Length (4 bytes) - The length of the payload in bytes.
// 3. Payload (variable length) - The actual data of the message, which can vary in size depending on the type of message.


// This class represents a proposal frame in the Paxos protocol.
// 1. Signature: "PROP"
// 2. Payload Length: 24 bytes
// 3. Payload:
//      - Proposer ID (4 bytes)
//      - Proposal ID (8 bytes)
//      - Value (4 bytes)
//      - Timestamp (8 bytes)

class ProposalFrame : public ns3::Header
{
public:
    ProposalFrame();
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;

    void Print(std::ostream &os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    // Constructor to initialize the proposal frame with proposer ID, proposal ID, value, and timestamp
    ProposalFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp);
    ~ProposalFrame();

    // Getters for the proposal frame fields
    uint32_t GetProposerId() const;
    uint64_t GetProposalId() const;
    uint32_t GetValue() const;
    ns3::Time GetTimestamp() const;
private:
    // Payload fields
    uint32_t m_proposerId; // ID of the proposer
    uint64_t m_proposalId; // ID of the proposal
    uint32_t m_value; // Value of the proposal
    ns3::Time m_timestamp; // Timestamp of the proposal
};

// This class represents an accept frame in the Paxos protocol.
// 1. Signature: "ACPT"
// 2. Payload Length: 24 bytes
// 3. Payload:
//      - Acceptor ID (4 bytes) // FFFF means that the acceptor is current failed
//      - Proposal ID (8 bytes)
//      - Node ID (4 bytes)
//      - Timestamp (8 bytes)

class AcceptFrame : public ns3::Header 
{
public:
    AcceptFrame();
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;

    void Print(std::ostream &os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    AcceptFrame(uint32_t acceptorId, uint64_t proposalId, uint32_t nodeId, ns3::Time timestamp);
    ~AcceptFrame();
    
    // Getters for the accept frame fields
    uint32_t GetAcceptorId() const;
    uint64_t GetProposalId() const;
    uint32_t GetNodeId() const;
    ns3::Time GetTimestamp() const;
private:
    uint32_t m_acceptorId; // ID of the acceptor
    uint64_t m_proposalId; // ID of the proposal
    uint32_t m_nodeId; // ID of the node that sent the accept message
    ns3::Time m_timestamp; // Timestamp of the accept message
};

// This class represents a decision frame in the Paxos protocol.
// 1. Signature: "DECI"
// 2. Payload Length: 24 bytes
// 3. Payload:
//     - Proposer ID (4 bytes)
//     - Proposal ID (8 bytes)
//     - Value (4 bytes)
//     - Timestamp (8 bytes)
class DecisionFrame : public ns3::Header 
{
public:
    DecisionFrame();
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;

    void Print(std::ostream &os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    DecisionFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp);
    ~DecisionFrame();

    uint32_t GetProposerId() const;
    uint64_t GetProposalId() const;
    uint32_t GetValue() const;
    ns3::Time GetTimestamp() const;
private:
    uint32_t m_proposerId; // ID of the proposer
    uint64_t m_proposalId; // ID of the proposal
    uint32_t m_value; // Value of the decision
    ns3::Time m_timestamp; // Timestamp of the decision
};


#endif // PAXOS_FRAME_HH
// End of paxos-frame.hh
