#ifndef PAXOS_APP_HH
#define PAXOS_APP_HH

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"

#include "common.hh"

#include <unordered_map>
#include <queue>

/**
 * \ingroup paxos
 * \brief Paxos application class that implements the Paxos consensus algorithm.
 *
 * This class represents a Paxos application that can act as a proposer or an acceptor.
 * It handles message sending and receiving, and manages the state of the Paxos protocol.
 */

class PaxosApp : public ns3::Application
{
public:
    PaxosApp(uint32_t selfId, NodeInfoList nodes);   // Constructor
    virtual ~PaxosApp();

    static ns3::TypeId GetTypeId(void);
    virtual void StartApplication(void);
    virtual void StopApplication(void);
    void SetNodeId(uint32_t nodeId);
    uint32_t GetNodeId() const;

    void CreateSendSocket();
    void CreateRecvSocket();

    void StartAcceptorThread();
    void StartProposerThread();

    void ReceiveMessage(ns3::Ptr<ns3::Packet> packet, const ns3::Address& from);
    void SendProposeToAll(std::shared_ptr<Proposal> proposal);
    //void SendProposeMessage(const ns3::Address& to, ns3::Ptr<ns3::Packet> packet);
    void SendAcceptMessage(std::shared_ptr<Proposal> proposal);
    void SendDecisionMessage(std::shared_ptr<Proposal> proposal);

    // Function to handle incoming messages
    void DoReceivedProposalMessage(ns3::Ptr<ProposalFrame> frame);
    void DoReceivedAcceptMessage(ns3::Ptr<AcceptFrame> frame);
    void DoReceivedDecisionMessage(ns3::Ptr<DecisionFrame> frame);

private:
    uint32_t m_selfID;  // Node ID of this node
    uint32_t m_numNodes;    // Number of nodes in the network
    NodeInfoList m_nodes; // List of all nodes in the network }; 

    ns3::Ptr<ns3::UdpSocket> m_recvSocket; // UDP socket for receiving messages
    ns3::Ptr<ns3::UdpSocket> m_sendSocket; // UDP socket for sending messages

    // proposals
    std::unorederded_map<uint64_t, std::shared_ptr<Proposal>> m_proposals; // Map of proposals
    uint64_t m_nextProposalId; // Next proposal ID to be used
    std::queue<std::shared_ptr<Proposal>> m_abandonedProposals; // Queue of abandoned proposals
    std::queue<std::shared_ptr<Proposal>> m_decidedProposals; // Queue of decided proposals
};

#endif // PAXOS_APP_HH

