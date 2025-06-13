#include "paxos-app-server.h"

NS_LOG_COMPONENT_DEFINE("PaxosAppServerProposer");

void PaxosAppServer::StartProposerThread()
{
    NS_LOG_INFO("Starting Proposer Thread");

    if (s_async)
    {
        // This is the asynchronous mode
        // Check if I am the leader
        if (m_serverId == s_leader)
        {
            // I am the leader
            // Do Propose
            m_leaderState = PAXOS_LEADER_WAITING_REQUEST;
            DoSyncPropose();
        }
        else
        {
            // I am not the leader
            // Do nothing
        }
    }
    else
    {
        // This is the synchronous mode
        // Calculate when to start the proposer
        m_proposeEvent = ns3::Simulator::Schedule(m_serverId * m_proposePeriod, &PaxosAppServer::DoSyncPropose, this);
    }
}

void PaxosAppServer::StopProposerThread()
{
    NS_LOG_INFO("Stopping Proposer Thread");
    // Close the m_proposeEvent
    if (m_proposeEvent.IsPending())
    {
        m_proposeEvent.Cancel();
    }
}

int32_t
PaxosAppServer::DoSyncPropose()
{
    NS_LOG_INFO("PaxosAppServer " << m_serverId << " DoSyncPropose");
    // If App is all ready stopped, do not propose
    if (ns3::Simulator::Now() >= m_stopTime)
    {
        NS_LOG_INFO("App is all ready stopped, do not propose");
        return 0;
    }

    DoPropose();

    // Schedule the next proposer thread to run after a certain interval
    m_proposeEvent = ns3::Simulator::Schedule(m_proposePeriod, &PaxosAppServer::DoSyncPropose, this);

    return 0;
}

int32_t
PaxosAppServer::DoAsyncPropose()
{
    NS_LOG_INFO("PaxosAppServer " << m_serverId << " DoAsyncPropose");

    // If App is all ready stopped, do not propose
    if (ns3::Simulator::Now() >= m_stopTime)
    {
        NS_LOG_INFO("App is all ready stopped, do not propose");
        return 0;
    }

    if (!m_waitingProposals.empty())
    {
        uint64_t proposalId = DoPropose();
        m_leaderState = PAXOS_LEADER_PROPOSED;

        // Set a timer to check if there is a timeout
        ns3::Simulator::Schedule(s_proposeTimeout, &PaxosAppServer::proposeTimerExpired, this, proposalId);
    }
    else
    {
        // There is no proposal in the queue
        ns3::Simulator::Schedule(ns3::NanoSeconds(10), &PaxosAppServer::DoAsyncPropose, this);
    }

    return 0;
}

uint64_t PaxosAppServer::DoPropose()
{
    if (m_waitingProposals.size() == 0)
    {
        // There is no proposal in the queue
        NS_LOG_INFO("PaxosAppServer " << m_serverId << " has no proposal in the queue.");
        return 0;
    }

    // Propose
    // 1. Check the waiting proposals
    // There are proposals in the queue
    NS_LOG_INFO("PaxosAppServer " << m_serverId << " has " << m_waitingProposals.size() << " proposals in the queue.");

    // Get the first proposal in the queue
    std::shared_ptr<Proposal> proposal = m_waitingProposals.front();

    // Do Propose
    proposal->setNodeId(m_serverId);
    proposal->setProposeTime(ns3::Simulator::Now());
    proposal->setNumAck(1);
    proposal->setPropState(Proposal::TO_BE_ACCEPTED);

    // Send Proposal to all nodes
    // Packet Header
    PaxosFrame proposalFrame;
    proposalFrame.SetMessageType(PaxosFrame::PROPOSAL);
    proposalFrame.SetProposalId(proposal->getProposalId());
    proposalFrame.SetProposerId(m_serverId);
    proposalFrame.SetValue(proposal->getValue());
    proposalFrame.SetProposeTime(ns3::Simulator::Now());

    NS_LOG_INFO("PaxosAppServer " << m_serverId << " created Proposal ID " << proposal->getProposalId() << " Propose Time " << proposal->getProposeTime());

    // Create Packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(proposalFrame);

    NS_LOG_INFO("PaxosAppServer " << m_serverId << " sending Proposal ID " << proposal->getProposalId() << " to all nodes.");
    // Send Proposal to all nodes
    for (auto node : m_nodes)
    {
        if (node.serverId == m_nodeId)
        {
            // Do not send the proposal to itself
            continue;
        }

        ns3::Ipv4Address address = node.address;
        uint16_t port = node.paxosPort;

        // Send the packet to the node
        ns3::InetSocketAddress to(address, port);

        if (m_sendSocket)
        {
            m_sendSocket->SendTo(packet, 0, to);
        }
        else
        {
            NS_FATAL_ERROR("SendSocket is null");
        }
    }

    m_proposals[proposal->getProposalId()] = proposal;
    // Remove the proposal from the queue
    m_waitingProposals.pop();

    return proposal->getProposalId();
}

void PaxosAppServer::proposeTimerExpired(uint64_t proposalId)
{
    // Check if the proposal is still in the queue
    if (m_proposals.find(proposalId) != m_proposals.end() &&
        m_leaderState == PAXOS_LEADER_PROPOSED)
    {
        // The proposal is still in the queue
        NS_LOG_INFO("PaxosAppServer " << m_serverId << " proposal ID " << proposalId << " is still in the queue.");

        // Repropose
        DoAsyncPropose();
    }
    else
    {
        // The proposal is not in the queue
        NS_LOG_INFO("PaxosAppServer " << m_serverId << " proposal ID " << proposalId << " is processed.");
    }
}
