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
#define MESSAGE_DELAY_BOUND (1000000) // Nanoseconds

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

// Define a PaxosConfig struct
typedef struct PaxosConfig {
    // 1. System mode
    bool isSynchronous = true; // true: synchronous; false: asynchronous

    // 2. Network Status
    // Only for synchronous mode
    std::string clockSyncError = "10ns";   // time synchronization error
    std::string boundedMessageDelay = "10ms"; // bounded message delay

    // Only for asynchronous mode
    std::string linkDelay = "10ms";       // link delay
    double packetLossRate = 0.0;          // packet loss rate

    // 3. Node Failure Rate
    double nodeFailureRate = 0.0;         // node failure rate (e.g. 0.01 means 1% failure rate)

    // 4. Paxos Config File Path
    std::string configFilePath = "";
} PaxosConfig;

class Proposal {
public:
    enum PropState {
        TO_BE_PROPOSED = 100,
        TO_BE_ACCEPTED,
        TO_BE_DECIDED
    } ;

    Proposal();
    Proposal(uint64_t proposalId, uint32_t serverId, ns3::Time proposeTime, ns3::Time acceptTime);
    ~Proposal();

    bool operator>(const Proposal& other) const;

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

    void setProposerId(uint32_t proposerId);
    uint32_t getProposerId();

    void setCreateTime(ns3::Time createTime);
    ns3::Time getCreateTime();

    void setReceiveTime(ns3::Time receiveTime);
    ns3::Time getReceiveTime();

    void setPropState(PropState propState);
    PropState getPropState();

    void setDecisionTime(ns3::Time decisionTime);
    ns3::Time getDecisionTime();

private:
    uint64_t m_proposalId;      // ID of the proposal, usually the timestamp
    uint32_t m_nodeId;          // ID of the server that propose the proposal
    uint32_t m_proposerId;      // ID of the server that propose the proposal

    ns3::Time m_createTime;     // Time when client create the proposal
    ns3::Time m_receiveTime;    // Time when server receive the proposal

    ns3::Time m_proposeTime;    // Time when server propose the proposal
    ns3::Time m_acceptTime;     // Time when get majority of accept
    ns3::Time m_decisionTime;   // Time when get majority of accept

    uint32_t m_value;           // Value of the proposal
    uint32_t m_numAck;          // Number of servers that accept the proposal
    
    PropState m_propState;      // State of the proposal
};


#endif // COMMON_HH