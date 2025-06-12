#ifndef PAXOS_APP_HH
#define PAXOS_APP_HH

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/socket.h"

#include "paxos-common.h"
#include "paxos-frame.h"

#include <unordered_map>
#include <queue>

/**
 * \ingroup paxos
 * \brief Paxos application class that implements the Paxos consensus algorithm.
 *
 * This class represents a Paxos application that can act as a proposer or an acceptor.
 * It handles message sending and receiving, and manages the state of the Paxos protocol.
 */

struct ProposalMinHeapCompare {
    bool operator()(std::shared_ptr<Proposal> p1, std::shared_ptr<Proposal> p2) const {
        // For a min-heap, return true if p1 should come AFTER p2 (i.e., p1 is "larger" than p2)
        return p1->getProposalId() > p2->getProposalId();
    }
};

class PaxosAppServer : public ns3::Application
{
public:
    PaxosAppServer();  // Default constructor
    PaxosAppServer(uint32_t selfId, NodeInfoList nodes);   // Constructor
    ~PaxosAppServer();

    static ns3::TypeId GetTypeId(void);
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void SetNodeId(uint32_t serverId);
    uint32_t GetNodeId() const;

    void CreateSendSocket();
    void CreateRecvSocket();

    // Listener Functions
    void ReceiveRequest(ns3::Ptr<ns3::Socket> socket);
    void StartListenerThread();
    void StopListenerThread();
    void CreateProposalFromRequest(RequestFrame requestFrame);

    // Proposer Functions
    void StartProposerThread();
    void StopProposerThread();
    int32_t DoPropose();

    // Acceptor Functions
    void StartAcceptorThread();
    void StopAcceptorThread();
    void ReceiveMessage(ns3::Ptr<ns3::Socket> socket);
    void SendAcceptMessage(PaxosFrame frame);
    void SendDecisionMessage(PaxosFrame frame);

    // Function to handle incoming messages
    void DoReceivedProposalMessage(PaxosFrame frame);
    void DoReceivedAcceptMessage(PaxosFrame frame);
    void DoReceivedDecisionMessage(PaxosFrame frame);

    // Configuration Function
    void SetClockSyncError(ns3::Time clockSyncError);
    void SetBoundedMessageDelay(ns3::Time boundedMessageDelay);
    void SetNodeFailureRate(double nodeFailureRate);

private:
    uint32_t m_nodeId;  // Node ID of this node
    uint32_t m_serverId; // Use serverId instead of nodeId
    uint32_t m_numNodes;    // Number of nodes in the network
    NodeInfoList m_nodes; // List of all nodes in the network }; 

    ns3::Time m_proposePeriod; // Period between proposing

    ns3::Ptr<ns3::Socket> m_recvSocket; // UDP socket for receiving messages
    ns3::Ptr<ns3::Socket> m_sendSocket; // UDP socket for sending messages

    // proposals
    ns3::EventId m_proposeEvent; // Event ID for proposing
    std::unordered_map<uint64_t, std::shared_ptr<Proposal>> m_proposals; // Map of proposing proposals
    std::unordered_map<uint64_t, std::shared_ptr<Proposal>> m_acceptedProposals; // Map of accepted proposals
    uint64_t m_nextProposalId; // Next proposal ID to be used
    std::queue<std::shared_ptr<Proposal>> m_abandonedProposals; // Queue of abandoned proposals
    //std::queue<std::shared_ptr<Proposal>> m_decidedProposals; // Queue of decided proposals
    std::priority_queue<std::shared_ptr<Proposal>,
        std::vector<std::shared_ptr<Proposal>>,
        ProposalMinHeapCompare> m_decidedProposalQueue;

    // Listener thread
    ns3::Ptr<ns3::Socket> m_listenerSocket; // UDP socket for listening for messages
    std::queue<std::shared_ptr<Proposal>> m_waitingProposals; // Queue of waiting proposals

    // Configuration
    ns3::Time m_clockSyncError; // Maximum clock synchronization error
    ns3::Time m_boundedMessageDelay; // Maximum message delay

    double m_nodeFailureRate;
};

#endif // PAXOS_APP_HH
