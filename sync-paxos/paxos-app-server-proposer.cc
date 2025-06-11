#include "paxos-app-server.h"

NS_LOG_COMPONENT_DEFINE("PaxosAppServerProposer");

void
PaxosAppServer::StartProposerThread()
{
    // TODO: Implement Proposer Thread logic
    //NS_LOG_DEBUG("Starting Proposer Thread for Node " << m_nodeId);          

    // First check if it is my turn to propose
    // If it is my turn, create a proposal and send it to all nodes
    // If it is not my turn, wait for the next round

    ns3::Time now = ns3::Simulator::Now();

    // Now the interval is 1 milisecond, the number of nodes is m_numNodes,
    // use a simple round-robin approach to determine the proposer
    if (m_nodeId == (now.GetMilliSeconds() % m_numNodes))
    {
        //NS_LOG_INFO("Node " << m_nodeId << " is the proposer for this round.");

        // Check if there is any proposal in the queue
        if (m_waitingProposals.size() > 0)
        {
            NS_LOG_INFO("There are " << m_waitingProposals.size() << " proposals in the queue.");
            // Get the first proposal in the queue
            std::shared_ptr<Proposal> proposal = m_waitingProposals.front();

            // Do Propose
            if (DoPropose(proposal) >= 0)
            {
                // Successfully proposed
                NS_LOG_INFO("Successfully proposed for Node " << m_nodeId);
                // Insert the proposal to the map
                m_proposals[proposal->getProposalId()] = proposal;

                // Remove the proposal from the queue
                m_waitingProposals.pop();
            }else
            {
                // Failed to propose
                NS_LOG_INFO("Failed to propose for Node " << m_nodeId);
            }
        }
    }
    else
    {
        //NS_LOG_INFO("Node " << m_nodeId << " is not the proposer for this round.");
    }

    // Schedule the next proposer thread to run after a certain interval
    ns3::Simulator::Schedule(ns3::MicroSeconds(10), &PaxosAppServer::StartProposerThread, this);

}

int32_t
PaxosAppServer::DoPropose(std::shared_ptr<Proposal> proposal)
{
    NS_LOG_INFO("Node " << m_nodeId << " is proposing for Proposal ID " << proposal->getProposalId());
    // fullfill the proposal
    proposal->setNodeId(m_nodeId);
    proposal->setProposeTime(ns3::Simulator::Now());
    proposal->setNumAck(1);

    // Send Proposal to all nodes
    // Packet Header
    PaxosFrame proposalFrame;
    proposalFrame.SetMessageType(PaxosFrame::PROPOSAL);
    proposalFrame.SetProposerId(m_nodeId);
    proposalFrame.SetProposalId(proposal->getProposalId());
    proposalFrame.SetValue(proposal->getValue());
    proposalFrame.SetProposeTime(ns3::Simulator::Now());

    // Create Packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(proposalFrame);

    // Add the proposal frame
    NS_LOG_INFO("Proposal Frame created with Proposer ID " << proposal->getNodeId()
                << ", Proposal ID " << proposal->getProposalId() 
                << ", Value " << proposal->getValue() 
                << ", Timestamp " << proposal->getProposeTime());

    // Send the proposal to all nodes
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

        m_sendSocket->SendTo(packet, 0, to);
    }
    NS_LOG_INFO("Proposal with ID " << proposal->getProposalId() << " sent to all nodes by Node " << m_nodeId);
    
    return 0;
}
