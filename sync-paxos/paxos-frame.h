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

#endif
