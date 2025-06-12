#ifndef COMMON_HH
#define COMMON_HH

#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/core-module.h"
#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/buffer.h"

#include <vector>

// Time Sync Error
// E1: 10 Nanoseconds   GPS & Atomic Clock
// E2: 100 Nanoseconds  GPS & Atomic Clock
// E3: 1000 Nanoseconds or 1 Microsecond
// E4: 10000 Nanoseconds or 10 Microseconds
// E5: 100000 Nanoseconds or 100 Microseconds
#define TIME_SYNC_ERROR (1000) // Nanoseconds

// Message Delay Bound
// L1: 10,000 Nanoseconds or 10 Microseconds
// L2: 100,000 Nanoseconds or 100 Microseconds
// L3: 1,000,000 Nanoseconds or 1 Millisecond
// L4: 10,000,000 Nanoseconds or 10 Milliseconds
// L5: 100,000,000 Nanoseconds or 100 Milliseconds
#define MESSAGE_DELAY_BOUND (100000) // Nanoseconds

#define PAXOS_PORT (9000)   // This port is used for Paxos protocol
#define SERVER_PORT (9001)  // This port is used for accept client reqeusts

#define LOG_DIR ("data/")

// Node ID and address
typedef struct {
    uint32_t serverId;
    ns3::Ipv4Address address;
    uint16_t paxosPort;
    uint16_t serverPort;
} NodeInfo;

typedef std::vector<NodeInfo> NodeInfoList;

class Proposal {
public:
    Proposal();
    Proposal(uint64_t proposalId, uint32_t serverId, ns3::Time proposeTime, ns3::Time acceptTime);
    ~Proposal();

    void setProposalId(uint64_t proposalId);
    void setNodeId(uint32_t serverId);
    uint64_t getProposalId();
    uint32_t getNodeId();
    void setProposeTime(ns3::Time proposeTime);
    void setAcceptTime(ns3::Time acceptTime);
    ns3::Time getProposeTime();
    ns3::Time getAcceptTime();

    void setValue(uint32_t value);
    uint32_t getValue();
    void setNumAck(uint32_t numAck);
    uint32_t getNumAck();
    void incrementNumAck();

private:
    uint64_t m_proposalId;
    uint32_t m_nodeId;

    ns3::Time m_proposeTime;
    ns3::Time m_acceptTime;   // accept time
    ns3::Time m_decisionTime;

    uint32_t m_value;
    uint32_t m_numAck;
};

bool isProposalSignature(uint8_t *data);
bool isAcceptSignature(uint8_t *data);
bool isDecisionSignature(uint8_t *data);


#endif // COMMON_HH