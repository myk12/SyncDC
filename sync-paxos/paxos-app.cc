#include "paxos-app.h"
#include "paxos-frame.h"

PaxosApp::PaxosApp(uint32_t selfId, NodeInfoList nodes)
{
    m_selfID = selfId;
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
        .AddConstructor<PaxosApp>()
        .AddAttribute("NodeId", "Node ID", uint32_t(0), MakeUint32Accessor(&PaxosApp::m_nodeId), MakeUint32Checker<uint32_t>())
        .AddAttribute("NumNodes", "Number of nodes in the network", uint32_t(0), MakeUint32Accessor(&PaxosApp::m_numNodes), MakeUint32Checker<uint32_t>());
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
    NS_LOG_INFO("Starting PaxosApp for Node " << m_selfID);
    ns3::Simulator::ScheduleNow(&PaxosApp::StartAcceptorThread, this);

    // Start Proposer Thread
    NS_LOG_INFO("Starting Proposer Thread for Node " << m_selfID);
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
    m_sendSocket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
    if (!m_sendSocket)
    {
        NS_LOG_ERROR("Failed to create send socket for PaxosApp " << m_selfID);
        return;
    }
}

void
PaxosApp::CreateRecvSocket()
{
    // Create a UDP socket for receiving messages
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
    if (!m_recvSocket)
    {
        NS_LOG_ERROR("Failed to create receive socket for PaxosApp " << m_selfID);
        return;
    }

    // Bind the socket to port 10000
    ns3::Address local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), 10000);
    if (m_recvSocket->Bind(local) == -1)
    {
        NS_LOG_ERROR("Failed to bind receive socket for PaxosApp " << m_selfID);
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

        // Add the proposal to the queue
        m_proposals.push(proposal);
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
PaxosApp::ReceiveMessage(Ptr<Packet> packet, const Address& from)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " received a message from " << InetSocketAddress::ConvertFrom(from).GetIpv4());
    // Process the received message
    // Parse frame type

    // Get Signature and Payload Length
    ns3::Buffer::Iterator start = packet->Begin();
    uint32_t signature = start.ReadU32(); // Read the signature
    uint32_t payloadLength = start.ReadU32(); // Read the payload length
    NS_LOG_INFO("Received packet with Signature: " << signature << ", Payload Length: " << payloadLength);
    // Check the signature to determine the type of frame
    if (signature == 0x50524F43) // "PROP"
    {
        // Parse the proposal frame
        ns3::Ptr<ProposalFrame> frame = Create<ProposalFrame>();
        packet->RemoveHeader(*frame);
        NS_LOG_INFO("Proposal Frame parsed with Proposer ID " << frame->GetProposerId() 
                    << ", Proposal ID " << frame->GetProposalId() 
                    << ", Value " << frame->GetValue() 
                    << ", Timestamp " << frame->GetTimestamp());

        // Schedule the proposal to be processed
        ns3::Simulator::ScheduleNow(&PaxosApp::DoReceivedProposalMessage, this, frame);
    }
    else if (signature == 0x41435054) // "ACPT"
    {
        // Parse the accept frame
        ns3::Ptr<AcceptFrame> frame = Create<AcceptFrame>();
        packet->RemoveHeader(*frame);
        NS_LOG_INFO("Accept Frame parsed with Acceptor ID " << frame->GetAcceptorId() 
                    << ", Proposal ID " << frame->GetProposalId() 
                    << ", Node ID " << frame->GetNodeId() 
                    << ", Timestamp " << frame->GetTimestamp());

        // Schedule the proposal to be processed
        ns3::Simulator::ScheduleNow(&PaxosApp::DoReceivedAcceptMessage, this, frame);
    }
    else if (signature == 0x44454349) // "DECI"
    {
        // Parse the decision frame
        ns3::Ptr<DecisionFrame> frame = Create<DecisionFrame>();
        packet->RemoveHeader(*frame);
        NS_LOG_INFO("Decision Frame parsed with Decision ID " << frame->GetDecisionId() 
                    << ", Value " << frame->GetValue() 
                    << ", Timestamp " << frame->GetTimestamp());
        
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

void
PaxosApp::DoReceivedProposalMessage(ns3::Ptr<ProposalFrame> frame)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " received a proposal from Proposer ID " << frame->GetProposerId() 
                << ", Proposal ID " << frame->GetProposalId() 
                << ", Value " << frame->GetValue() 
                << ", Timestamp " << frame->GetTimestamp());

    // Check if the proposer is the right one
    uint32_t proposerId = frame->GetProposerId();
    if (proposerId != ns3::Simulator::Now().GetMilliSeconds() % m_numNodes)
    {
        NS_LOG_WARN("PaxosApp " << m_nodeId << " ignoring proposal from wrong proposer ID " << proposerId);
        return; // Ignore the proposal if it is not from the right proposer
    }

    // Process the proposal
    NS_LOG_INFO("PaxosApp " << m_nodeId << " processing proposal with ID " << frame->GetProposalId());

    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>(frame->GetProposerId(), frame->GetProposalId(), frame->GetValue(), frame->GetTimestamp());
    ns3::Simulator::ScheduleNow(&PaxosApp::SendAcceptMessage, this, proposal);
}

void
PaxosApp::SendAcceptMessage(std::shared_ptr<Proposal> proposal)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " sending accept message for Proposal ID " << proposal->getProposalId());

    // Create a new packet
    ns3::Ptr<ns3::Packet> packet = ns3::Packet::Create();

    // TODO: Add failure model to simulate network failures
    // For now, we will just log the packet creation
    // Add Signature and Payload Length
    // For simplicity, we assume the signature is a fixed string "ACPT"
    ns3::Buffer::Iterator start = packet->Begin();
    start.WriteU32(0x41435054); // Signature "ACPT"
    start.WriteU32(24);         // Payload Length

    // Add the accept frame
    ns3::Ptr<ns3::Header> header = Create<AcceptFrame>(m_nodeId, proposal->getProposalId(), proposal->getNodeId(), proposal->getProposeTime());
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
PaxosApp::DoReceivedAcceptMessage(ns3::Ptr<AcceptFrame> frame)
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
    proposal->incrementAcceptCount();

    // If the proposal has enough accepts, send a decision message
    if (proposal->getAcceptCount() >= (m_numNodes / 2))
    {
        NS_LOG_INFO("PaxosApp " << m_nodeId << " sending decision message for Proposal ID " << proposal->getProposalId());

        // Create a decision frame and send it to all nodes
        ns3::Ptr<DecisionFrame> decisionFrame = Create<DecisionFrame>(proposal->getProposalId(), proposal->getValue(), proposal->getProposeTime());
        SendDecisionMessage(ns3::InetSocketAddress(ns3::Ipv4Address::GetBroadcast(), 10000), decisionFrame);

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

    // Create a new packet
    ns3::Ptr<ns3::Packet> packet = ns3::Packet::Create();

    // Add Signature and Payload Length
    // For simplicity, we assume the signature is a fixed string "DECI"
    ns3::Buffer::Iterator start = packet->Begin();
    start.WriteU32(0x44454349); // Signature "DECI"
    start.WriteU32(24);         // Payload Length

    // Add the decision frame
    ns3::Ptr<ns3::Header> header = Create<DecisionFrame>(m_nodeId, proposal->getProposalId(), proposal->getValue(), proposal->getProposeTime());
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
PaxosApp::DoReceivedDecisionMessage(ns3::Ptr<DecisionFrame> frame)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " received a decision message from Decision ID " << frame->GetDecisionId() 
                << ", Value " << frame->GetValue() 
                << ", Timestamp " << frame->GetTimestamp());

    uint64_t proposalId = frame->GetDecisionId();
    uint32_t value = frame->GetValue();

    ns3::Time timestamp = frame->GetTimestamp();

    // Add the decision to the decided proposals queue
    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>(m_nodeId, proposalId, value, timestamp);
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
PaxosApp::SendMessage(const Address& to, Ptr<Packet> packet)
{
    NS_LOG_INFO("PaxosApp " << m_nodeId << " sending message to " << InetSocketAddress::ConvertFrom(to).GetIpv4());
    // Send the packet to the specified address
    ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket(GetNode(), ns3::UdpSocketFactory::GetTypeId());
    socket->SendTo(packet, 0, to);
}

void
PaxosApp::SendProposeToAll(std::shared_ptr<Proposal> proposal)
{
    // Create a new packet
    ns3::Ptr<ns3::Packet> packet = ns3::Packet::Create();

    // Add Signature and Payload Length
    // For simplicity, we assume the signature is a fixed string "PROP"
    ns3::Buffer::Iterator start = packet->Begin();
    start.WriteU32(0x50524F43); // Signature "PROP"
    start.WriteU32(24);         // Payload Length

    // Add the proposal frame
    ns3::Ptr<ns3::Header> header = Create<ProposalFrame>(proposal->getProposerId(), proposal->getProposalId(), proposal->getValue(), proposal->getProposeTime());
    packet->AddHeader(*header);
    NS_LOG_INFO("Proposal Frame created with Proposer ID " << proposal->getProposerId() 
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
    NS_LOG_INFO("Proposal with ID " << proposal->getProposalId() << " sent to all nodes by Node " << m_selfID);
}


