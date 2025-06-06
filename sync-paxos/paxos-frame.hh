#ifndef PAXOS_FRAME_HH
#define PAXOS_FRAME_HH

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"


// This class define the frame format for Paxos messages.
// It is used to encapsulate the messages sent between nodes in the Paxos protocol.
// The general structure of the frame is as follows:    
// 1. Signature (4 bytes) - A unique identifier for the message type.
//    - "PROP" for Proposal messages
//    - "ACPT" for Accept messages
//    - "DECI" for Decision messages

// 2. Payload Length (4 bytes) - The length of the payload in bytes.
// 3. Payload (variable length) - The actual data of the message, which can vary in size depending on the type of message.

class PaxosFrame : public ns3::Header
{
public:
    static ns3::TypeId GetTypeId(void);
    virtual void Print(std::ostream &os) const;
    virtual ns3::TypeId GetInstanceTypeId(void) const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(ns3::Buffer::Iterator start) const;
    virtual uint32_t Deserialize(ns3::Buffer::Iterator start);
    virtual void SetSignature(uint32_t signature) { m_signature = signature; }
    virtual uint32_t GetSignature() const { return m_signature; }
    virtual void SetPayloadLength(uint32_t payloadLength) { m_payloadLength = payloadLength; }
    virtual uint32_t GetPayloadLength() const { return m_payloadLength; }

private:
    uint32_t m_signature;  // Signature to identify the message type
    uint32_t m_payloadLength; // Length of the payload
};

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
    static ns3::TypeId GetTypeId(void);

    virtual void Print(std::ostream &os) const override;
    virtual ns3::TypeId GetInstanceTypeId(void) const override;
    virtual uint32_t GetSerializedSize(void) const override;
    virtual void Serialize(ns3::Buffer::Iterator start) const override;
    virtual uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    // Constructor to initialize the proposal frame with proposer ID, proposal ID, value, and timestamp
    ProposalFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp);

    // Getters for the proposal frame fields
    uint32_t GetProposerId() const { return m_proposerId; }
    uint64_t GetProposalId() const { return m_proposalId; }
    uint32_t GetValue() const { return m_value; }
    ns3::Time GetTimestamp() const { return m_timestamp; }
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
    static ns3::TypeId GetTypeId(void);
    virtual void Print(std::ostream &os) const override;
    virtual ns3::TypeId GetInstanceTypeId(void) const override;
    virtual uint32_t GetSerializedSize(void) const override;
    virtual void Serialize(ns3::Buffer::Iterator start) const override;
    virtual uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    AcceptFrame(uint32_t acceptorId, uint64_t proposalId, uint32_t nodeId, ns3::Time timestamp);
    
    uint32_t GetAcceptorId() const { return m_acceptorId; }
    uint64_t GetProposalId() const { return m_proposalId; }
    uint32_t GetNodeId() const { return m_nodeId; }
    ns3::Time GetTimestamp() const { return m_timestamp; }
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
    static ns3::TypeId GetTypeId(void);
    virtual void Print(std::ostream &os) const override;
    virtual ns3::TypeId GetInstanceTypeId(void) const override;
    virtual uint32_t GetSerializedSize(void) const override;
    virtual void Serialize(ns3::Buffer::Iterator start) const override;
    virtual uint32_t Deserialize(ns3::Buffer::Iterator start) override;

    DecisionFrame(uint32_t proposerId, uint64_t proposalId, uint32_t value, ns3::Time timestamp)

    uint32_t GetProposerId() const { return m_proposerId; }
    uint64_t GetProposalId() const { return m_proposalId; }
    uint32_t GetValue() const { return m_value; }
    ns3::Time GetTimestamp() const { return m_timestamp; }
private:
    uint32_t m_proposerId; // ID of the proposer
    uint64_t m_proposalId; // ID of the proposal
    uint32_t m_value; // Value of the decision
    ns3::Time m_timestamp; // Timestamp of the decision
};


#endif // PAXOS_FRAME_HH
// End of paxos-frame.hh
