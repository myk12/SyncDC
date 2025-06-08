#include "paxos-app.h"
#include "paxos-frame.h"

// define LOG
NS_LOG_COMPONENT_DEFINE("PaxosApp");

PaxosApp::PaxosApp()
    : m_nodeId(0), m_numNodes(0), m_nextProposalId(0)
{
    NS_LOG_FUNCTION(this);
}

PaxosApp::PaxosApp(uint32_t selfId, NodeInfoList nodes)
{
    m_nodeId = selfId;
    m_numNodes = nodes.size();
    m_nodes = nodes;
}

PaxosApp::~PaxosApp()
{
}

ns3::TypeId
PaxosApp::GetTypeId(void)
{
    static ns3::TypeId tid = ns3::TypeId("PaxosApp")
        .SetParent<ns3::Application>()
        .AddConstructor<PaxosApp>();
        return tid;
}

void
PaxosApp::StartApplication(void)
{
    // Create a UDP socket for receiving messages
    CreateRecvSocket();

    // Create a UDP socket for sending messages
    CreateSendSocket();

    // Start Acceptor Thread
    NS_LOG_INFO("Starting PaxosApp for Node " << m_nodeId);
    ns3::Simulator::ScheduleNow(&PaxosApp::StartAcceptorThread, this);

    // Start Proposer Thread
    NS_LOG_INFO("Starting Proposer Thread for Node " << m_nodeId);
    ns3::Simulator::ScheduleNow(&PaxosApp::StartProposerThread, this);
}

void
PaxosApp::StopApplication(void)
{
    // TODO: Implement this method
}

void
PaxosApp::SetNodeId(uint32_t nodeId)
{
    m_nodeId = nodeId;
}

uint32_t
PaxosApp::GetNodeId() const
{
    return m_nodeId;
}

void
PaxosApp::CreateSendSocket()
{
    // Create a UDP socket for sending messages
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_sendSocket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (!m_sendSocket)
    {
        NS_LOG_ERROR("Failed to create send socket for PaxosApp " << m_nodeId);
        return;
    }
}

void
PaxosApp::CreateRecvSocket()
{
    // Create a UDP socket for receiving messages
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (!m_recvSocket)
    {
        NS_LOG_ERROR("Failed to create receive socket for PaxosApp " << m_nodeId);
        return;
    }

    // Bind the socket to port 10000
    ns3::Address local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), 10000);
    if (m_recvSocket->Bind(local) == -1)
    {
        NS_LOG_ERROR("Failed to bind receive socket for PaxosApp " << m_nodeId);
        return;
    }

    // Set the receive callback
    m_recvSocket->SetRecvCallback(MakeCallback(&PaxosApp::ReceiveMessage, this));
}

void
PaxosApp::StartAcceptorThread()
{
    // 
}

void
PaxosApp::StartProposerThread()
{
    // TODO: Implement Proposer Thread logic
    NS_LOG_INFO("Starting Proposer Thread for Node " << m_nodeId);          

    // First check if it is my turn to propose
    // If it is my turn, create a proposal and send it to all nodes
    // If it is not my turn, wait for the next round

    ns3::Time now = ns3::Simulator::Now();

    // Now the interval is 1 milisecond, the number of nodes is m_numNodes,
    // use a simple round-robin approach to determine the proposer
    if (m_nodeId == (now.GetMilliSeconds() % m_numNodes))
    {
        NS_LOG_INFO("Node " << m_nodeId << " is the proposer for this round.");

        // Create a new proposal
        std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
        proposal->setProposalId(m_nextProposalId++);
        proposal->setNodeId(m_nodeId);
        proposal->setProposeTime(now);
        proposal->setValue(42); // Example value, can be set to any value
        proposal->setNumAck(0);
        NS_LOG_INFO("Created proposal with ID " << proposal->getProposalId() << " by Node " << m_nodeId);

        // Add proposal to the map
        m_proposals[proposal->getProposalId()] = proposal;
        NS_LOG_INFO("Proposal added to queue for Node " << m_nodeId);
        // Send the proposal to all nodes
        SendProposeToAll(proposal);
        NS_LOG_INFO("Proposal with ID " << proposal->getProposalId() << " sent to all nodes by Node " << m_nodeId);
    }
    else
    {
        NS_LOG_INFO("Node " << m_nodeId << " is not the proposer for this round.");
    }

    // Schedule the next proposer thread to run after a certain interval
    ns3::Simulator::Schedule(ns3::MicroSeconds(10), &PaxosApp::StartProposerThread, this);

}

void
PaxosApp::ReceiveMessage(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " receiving message on socket " << socket->GetNode()->GetId());
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;

    // Receive the packet
    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_INFO("PaxosApp " << m_nodeId << " received a packet of size " << packet->GetSize() 
                    << " from " << ns3::InetSocketAddress::ConvertFrom(from).GetIpv4());

        uint8_t signature[4];
        // Read the first 4 bytes to determine the signature
        packet->CopyData(signature, sizeof(signature));
        // Check the signature to determine the type of frame
        if (isProposalSignature(signature)) // "PROP"
        {
            // Parse the proposal frame
            std::shared_ptr<ProposalFrame> frame = std::make_shared<ProposalFrame>();
            packet->RemoveHeader(*frame);
            NS_LOG_INFO("Proposal Frame parsed with Proposer ID " << frame->GetProposerId() 
                        << ", Proposal ID " << frame->GetProposalId() 
                        << ", Value " << frame->GetValue() 
                        << ", Timestamp " << frame->GetTimestamp());

            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosApp::DoReceivedProposalMessage, this, frame);
        }
        else if (isAcceptSignature(signature)) // "ACPT"
        {
            // Parse the accept frame
            std::shared_ptr<AcceptFrame> frame = std::make_shared<AcceptFrame>();
            packet->RemoveHeader(*frame);
            NS_LOG_INFO("Accept Frame parsed with Acceptor ID " << frame->GetAcceptorId() 
                        << ", Proposal ID " << frame->GetProposalId() 
                        << ", Node ID " << frame->GetNodeId()
                        << ", Timestamp " << frame->GetTimestamp());
            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosApp::DoReceivedAcceptMessage, this, frame);
        }
        else if (isDecisionSignature(signature)) // "DECI"
        {
            // Parse the decision frame
            std::shared_ptr<DecisionFrame> frame = std::make_shared<DecisionFrame>();
            packet->RemoveHeader(*frame);
            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosApp::DoReceivedDecisionMessage, this, frame);
        }
        else
        {
            NS_LOG_ERROR("Unknown packet signature: " << signature);
            return; // Unknown packet type, ignore it
        }

        // For now, just log the packet size
        NS_LOG_INFO("Packet size: " << packet->GetSize());
    }
}

void
PaxosApp::DoReceivedProposalMessage(std::shared_ptr<ProposalFrame> frame)
{
    // Check if the proposer is the right one
    uint32_t proposerId = frame->GetProposerId();
    if (proposerId != ns3::Simulator::Now().GetMilliSeconds() % m_numNodes)
    {
        NS_LOG_WARN("PaxosApp " << m_nodeId << " ignoring proposal from wrong proposer ID " << proposerId);
        return; // Ignore the proposal if it is not from the right proposer
    }

    // Process the proposal
    NS_LOG_INFO("PaxosApp " << m_nodeId << " processing proposal with ID " << frame->GetProposalId());

    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
    proposal->setProposalId(frame->GetProposalId());
    proposal->setNodeId(frame->GetProposerId());
    proposal->setValue(frame->GetValue());
    proposal->setProposeTime(frame->GetTimestamp());
    proposal->setNumAck(0);

    ns3::Simulator::ScheduleNow(&PaxosApp::SendAcceptMessage, this, proposal);
}

void
PaxosApp::SendAcceptMessage(std::shared_ptr<Proposal> proposal)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " sending accept message for Proposal ID " << proposal->getProposalId());

    // Create Packet with signature
    // Signature: "ACPT"
    uint8_t signature[4] = {0x41, 0x43, 0x50, 0x54}; // "ACPT"
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(signature, sizeof(signature));

    // Add the accept frame
    std::shared_ptr<AcceptFrame> header = std::make_shared<AcceptFrame>(
        m_nodeId, // Acceptor ID
        proposal->getProposalId(), // Proposal ID
        proposal->getNodeId(), // Node ID
        ns3::Simulator::Now() // Timestamp
    );
    // Add the header to the packet
    packet->AddHeader(*header);
    NS_LOG_INFO("Accept Frame created with Acceptor ID " << m_nodeId 
                << ", Proposal ID " << proposal->getProposalId() 
                << ", Node ID " << proposal->getNodeId() 
                << ", Timestamp " << proposal->getProposeTime());

    // Send the accept message to the proposer
    ns3::Ipv4Address address = m_nodes[proposal->getNodeId()].address;
    uint16_t port = m_nodes[proposal->getNodeId()].port;
    ns3::InetSocketAddress to(address, port);

    m_sendSocket->SendTo(packet, 0, to);
}

void
PaxosApp::DoReceivedAcceptMessage(std::shared_ptr<AcceptFrame> frame)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " received an accept message from Acceptor ID " << frame->GetAcceptorId() 
                << ", Proposal ID " << frame->GetProposalId() 
                << ", Node ID " << frame->GetNodeId() 
                << ", Timestamp " << frame->GetTimestamp());

    uint64_t proposalId = frame->GetProposalId();
    uint32_t acceptorId = frame->GetAcceptorId();
    uint32_t nodeId = frame->GetNodeId();
    ns3::Time timestamp = frame->GetTimestamp();

    if (nodeId != m_nodeId)
    {
        NS_LOG_WARN("PaxosApp " << m_nodeId << " ignoring accept message from wrong node ID " << nodeId);
        return; // Ignore the accept message if it is not from the right node
    }

    // Find the proposal in the map
    auto it = m_proposals.find(proposalId);
    if (it == m_proposals.end())
    {
        NS_LOG_WARN("PaxosApp " << m_nodeId << " ignoring accept message for unknown proposal ID " << proposalId);
        return; // Ignore the accept message if the proposal is not in the map
    }

    // Get the proposal
    std::shared_ptr<Proposal> proposal = it->second;

    // Increment the accept count
    proposal->incrementNumAck();

    // If the proposal has enough accepts, send a decision message
    if (proposal->getNumAck() >= (m_numNodes / 2))
    {
        NS_LOG_INFO("PaxosApp " << m_nodeId << " sending decision message for Proposal ID " << proposal->getProposalId());

        SendDecisionMessage(proposal);

        // Remove the proposal from the map
        m_proposals.erase(proposalId);
    }
    else
    {
        NS_LOG_INFO("PaxosApp " << m_nodeId << " proposal ID " << proposalId << " has not enough accepts yet.");
    }
}

void
PaxosApp::SendDecisionMessage(std::shared_ptr<Proposal> proposal)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " sending decision message for Proposal ID " << proposal->getProposalId());

    // Create Packet with signature
    // Signature: "DECI"
    uint8_t signature[4] = {0x44, 0x45, 0x43, 0x49}; // "DECI"
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(signature, sizeof(signature));

    // Add the decision frame
    std::shared_ptr<DecisionFrame> header = std::make_shared<DecisionFrame>(
        m_nodeId,
        proposal->getProposalId(),
        proposal->getValue(),
        ns3::Simulator::Now()
    );

    packet->AddHeader(*header);
    NS_LOG_INFO("Decision Frame created with Proposer ID " << m_nodeId 
                << ", Proposal ID " << proposal->getProposalId() 
                << ", Value " << proposal->getValue() 
                << ", Timestamp " << proposal->getProposeTime());

    // Send the decision message to all nodes
    for (auto node : m_nodes)
    {
        ns3::Ipv4Address address = node.address;
        uint16_t port = node.port;

        // Send the packet to the node
        ns3::InetSocketAddress to(address, port);

        m_sendSocket->SendTo(packet, 0, to);
    }
}

void
PaxosApp::DoReceivedDecisionMessage(std::shared_ptr<DecisionFrame> frame)
{
    uint64_t proposalId = frame->GetProposalId();
    uint32_t value = frame->GetValue();

    ns3::Time timestamp = frame->GetTimestamp();

    // Add the decision to the decided proposals queue
    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
    proposal->setProposalId(proposalId);
    proposal->setValue(value);
    proposal->setProposeTime(timestamp);
    proposal->setNodeId(m_nodeId); // Set the node ID to the current node
    proposal->setNumAck(0); // Reset the number of acknowledgments
    NS_LOG_INFO("PaxosApp " << m_nodeId << " received decision for Proposal ID " << proposalId 
                << " with Value " << value 
                << " at Timestamp " << timestamp);
    // Add the proposal to the decided proposals queue
    // TODO: Set the maximum size of the queue if needed
    m_decidedProposals.push(proposal);

    // Delete the proposal from the proposals map
    auto it = m_proposals.find(proposalId);
    if (it != m_proposals.end())
    {
        m_proposals.erase(it);
        NS_LOG_INFO("PaxosApp " << m_nodeId << " removed proposal ID " << proposalId << " from proposals map.");
    }
    else
    {
        NS_LOG_WARN("PaxosApp " << m_nodeId << " could not find proposal ID " << proposalId << " in proposals map.");
    }
    NS_LOG_INFO("PaxosApp " << m_nodeId << " added proposal ID " << proposalId << " to decided proposals queue with value " << value);
    // Log the decision
    NS_LOG_INFO("PaxosApp " << m_nodeId << " made a decision for Proposal ID " << proposalId 
                << " with Value " << value 
                << " at Timestamp " << timestamp);
}

void
PaxosApp::SendProposeToAll(std::shared_ptr<Proposal> proposal)
{
    // Create Packet with signature
    // Signature: "PROP"
    uint8_t signature[4] = {0x50, 0x52, 0x4F, 0x50}; // "PROP"
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>(signature, sizeof(signature));

    // Add the proposal frame
    ProposalFrame proposalFrame(proposal->getNodeId(), proposal->getProposalId(), proposal->getValue(), proposal->getProposeTime());
    packet->AddHeader(proposalFrame);
    NS_LOG_INFO("Proposal Frame created with Proposer ID " << proposal->getNodeId()
                << ", Proposal ID " << proposal->getProposalId() 
                << ", Value " << proposal->getValue() 
                << ", Timestamp " << proposal->getProposeTime());

    // Send the proposal to all nodes
    for (auto node : m_nodes)
    {
        ns3::Ipv4Address address = node.address;
        uint16_t port = node.port;

        // Send the packet to the node
        ns3::InetSocketAddress to(address, port);

        m_sendSocket->SendTo(packet, 0, to);
    }
    NS_LOG_INFO("Proposal with ID " << proposal->getProposalId() << " sent to all nodes by Node " << m_nodeId);
}


