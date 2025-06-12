#include "paxos-app-server.h"

NS_LOG_COMPONENT_DEFINE("PaxosAppServerProposer");

void
PaxosAppServer::StartProposerThread()
{
    NS_LOG_INFO("Starting Proposer Thread");
    // Calculate when to start the proposer
    // The period is numServers * (2*Time_SYNC_ERROR + MESSAGE_DELAY_BOUND)
    // The start time is now + serverId * (2*Time_SYNC_ERROR + MESSAGE_DELAY_BOUND)

    m_proposeEvent = ns3::Simulator::Schedule(m_serverId * m_proposePeriod, &PaxosAppServer::DoPropose, this);
}

void
PaxosAppServer::StopProposerThread()
{
    NS_LOG_INFO("Stopping Proposer Thread");
    // Close the m_proposeEvent
    if (m_proposeEvent.IsPending())
    {
        m_proposeEvent.Cancel();
    }
}

int32_t
PaxosAppServer::DoPropose()
{
    NS_LOG_INFO("PaxosAppServer " << m_serverId << " DoPropose");
    // If App is all ready stopped, do not propose
    if (ns3::Simulator::Now() >= m_stopTime)
    {
        NS_LOG_INFO("App is all ready stopped, do not propose");
        return 0;
    }

    // Propose
    // 1. Check the waiting proposals
    if (m_waitingProposals.size() > 0)
    {
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
            }else
            {
                NS_FATAL_ERROR("SendSocket is null");
            }
        }

        // Add to the m_proposals
        m_proposals[proposal->getProposalId()] = proposal;

        // Remove the proposal from the queue
        m_waitingProposals.pop();
    }else
    {
        // No proposals in the queue
        NS_LOG_INFO("No proposals in the queue.");
    }

    // Schedule the next proposer thread to run after a certain interval
    m_proposeEvent = ns3::Simulator::Schedule(m_proposePeriod, &PaxosAppServer::DoPropose, this);

    return 0;
}
