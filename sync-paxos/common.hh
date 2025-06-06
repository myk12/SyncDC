#ifndef COMMON_HH
#define COMMON_HH

#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/core-module.h"

#include <vector>

// Node ID and address
typedef struct {
    uint32_t nodeId;
    ns3::Ipv4Address address;
    uint16_t port;
} NodeInfo;

typedef std::vector<NodeInfo> NodeInfoList;

class Proposal {
public:
    Proposal();
    Proposal(uint64_t proposalId, uint32_t nodeId, ns3::Time proposeTime, ns3::Time acceptTime);
    ~Proposal();

    void setProposalId(uint64_t proposalId) { m_proposalId = proposalId; }
    void setNodeId(uint32_t nodeId) { m_nodeId = nodeId; }
    uint64_t getProposalId() { return m_proposalId; }
    uint32_t getNodeId() { return m_nodeId; }
    void setProposeTime(ns3::Time proposeTime) { m_proposeTime = proposeTime; }
    void setAcceptTime(ns3::Time acceptTime) { m_acceptTime = acceptTime; }
    ns3::Time getProposeTime() { return m_proposeTime; }
    ns3::Time getAcceptTime() { return m_acceptTime; }

    void setValue(uint32_t value) { m_value = value; }
    uint32_t getValue() { return m_value; }
    void setNumAck(uint32_t numAck) { m_numAck = numAck; }
    uint32_t getNumAck() { return m_numAck; }
    void incrementNumAck() { m_numAck++; }

private:
    uint64_t m_proposalId;
    uint32_t m_nodeId;

    ns3::Time m_proposeTime;
    ns3::Time m_acceptTime;   // accept time

    uint32_t m_value;
    uint32_t m_numAck;
};

#endif // COMMON_HH