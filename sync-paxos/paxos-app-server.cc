#include "paxos-app-server.h"
#include "paxos-frame.h"

#include <filesystem>

// define LOG
NS_LOG_COMPONENT_DEFINE("PaxosAppServer");

PaxosAppServer::PaxosAppServer()
    : m_nodeId(0), m_numNodes(0), m_nextProposalId(0)
{
    NS_LOG_FUNCTION(this);
}

PaxosAppServer::PaxosAppServer(uint32_t selfId, NodeInfoList nodes)
{
    m_nodeId = selfId;
    m_serverId = selfId;
    m_numNodes = nodes.size();
    m_nodes = nodes;
}

PaxosAppServer::~PaxosAppServer()
{
}

ns3::TypeId
PaxosAppServer::GetTypeId(void)
{
    static ns3::TypeId tid = ns3::TypeId("PaxosAppServer")
        .SetParent<ns3::Application>()
        .AddConstructor<PaxosAppServer>();
        return tid;
}

void
PaxosAppServer::SetClockSyncError(ns3::Time clockSyncError)
{
    m_clockSyncError = clockSyncError;
}

void
PaxosAppServer::SetBoundedMessageDelay(ns3::Time boundedMessageDelay)
{
    m_boundedMessageDelay = boundedMessageDelay;
}

void
PaxosAppServer::SetNodeFailureRate(double nodeFailureRate)
{
    m_nodeFailureRate = nodeFailureRate;
}

void
PaxosAppServer::StartApplication(void)
{
    // Create a UDP socket for receiving messages
    CreateRecvSocket();

    // Create a UDP socket for sending messages
    CreateSendSocket();

    // Calculate the proposal period
    m_proposePeriod = 2*m_clockSyncError + m_boundedMessageDelay;
    NS_LOG_INFO("Clock sync error: " << m_clockSyncError.GetNanoSeconds() << "ns, Bounded message delay: " << m_boundedMessageDelay.GetNanoSeconds() << "ns");
    NS_LOG_INFO("Proposal period: " << m_proposePeriod.GetNanoSeconds() << "ns");

    // Start Listener Thread
    NS_LOG_INFO("Starting Listener Thread for Node " << m_nodeId);
    ns3::Simulator::ScheduleNow(&PaxosAppServer::StartListenerThread, this);

    // Start Proposer Thread
    NS_LOG_INFO("Starting Proposer Thread for Node " << m_nodeId);
    ns3::Simulator::ScheduleNow(&PaxosAppServer::StartProposerThread, this);

    // Start Acceptor Thread
    NS_LOG_INFO("Starting Acceptor Thread for Node " << m_nodeId);
    ns3::Simulator::ScheduleNow(&PaxosAppServer::StartAcceptorThread, this);

}

void
PaxosAppServer::StopApplication(void)
{
    // Log the proposal to file
    // Write the proposal to file
    // Log file path : LOG_DIR + "server-" + m_nodeId + "-decision-log.dat"
    std::string logFilePath = "server-" + std::to_string(m_nodeId) + "-decision-log.dat";

    // Create file if it not exists
    if (!std::filesystem::exists(logFilePath))
    {
        std::ofstream logFile(logFilePath, std::ios::out);
        logFile.close();
    }

    std::ofstream logFile(logFilePath, std::ios::out);
    logFile <<"index,proposalId,proposerId,value,decidedTime\n";

    uint64_t length = m_decidedProposalQueue.size();
    for (uint64_t i=0; i<length; i++)
    {
        auto proposal = m_decidedProposalQueue.top();
        logFile << i << "," << proposal->getProposalId() << "," << proposal->getNodeId() << "," << proposal->getValue() << "," << proposal->getDecisionTime() << std::endl;
        m_decidedProposalQueue.pop();
    }

    logFile.close();
}

void
PaxosAppServer::SetNodeId(uint32_t serverId)
{
    m_nodeId = serverId;
}

uint32_t
PaxosAppServer::GetNodeId() const
{
    return m_nodeId;
}

void
PaxosAppServer::CreateSendSocket()
{
    // Create a UDP socket for sending messages
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_sendSocket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (!m_sendSocket)
    {
        NS_LOG_ERROR("Failed to create send socket for PaxosAppServer " << m_nodeId);
        return;
    }
}

void
PaxosAppServer::CreateRecvSocket()
{
    // Create a UDP socket for receiving messages
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_recvSocket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (!m_recvSocket)
    {
        NS_LOG_ERROR("Failed to create receive socket for PaxosAppServer " << m_nodeId);
        return;
    }

    // Bind the socket
    ns3::Address local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), m_nodes[m_nodeId].paxosPort);
    if (m_recvSocket->Bind(local) == -1)
    {
        NS_LOG_ERROR("Failed to bind receive socket for PaxosAppServer " << m_nodeId);
        return;
    }

    // Set the receive callback
    m_recvSocket->SetRecvCallback(MakeCallback(&PaxosAppServer::ReceiveMessage, this));
}

void
PaxosAppServer::StartAcceptorThread()
{
}

void
PaxosAppServer::StopAcceptorThread()
{
}

void
PaxosAppServer::ReceiveMessage(ns3::Ptr<ns3::Socket> socket)
{
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;

    // Receive the packet
    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_INFO("PaxosAppServer " << m_nodeId << " received a packet of size " << packet->GetSize() 
                    << " from " << ns3::InetSocketAddress::ConvertFrom(from).GetIpv4());

        PaxosFrame pktHeader;
        packet->RemoveHeader(pktHeader);

        if (pktHeader.IsProposal())
        {
            NS_LOG_INFO("PaxosAppServer " << m_nodeId << " receiving proposal message for Proposal ID " << pktHeader.GetProposalId());
            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosAppServer::DoReceivedProposalMessage, this, pktHeader);
        }
        else if (pktHeader.IsAccept())
        {
            NS_LOG_INFO("PaxosAppServer " << m_nodeId << " receiving accept message for Proposal ID " << pktHeader.GetProposalId());
            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosAppServer::DoReceivedAcceptMessage, this, pktHeader);
        }
        else if (pktHeader.IsDecision())
        {
            NS_LOG_INFO("PaxosAppServer " << m_nodeId << " receiving decision message for Proposal ID " << pktHeader.GetProposalId());
            // Schedule the proposal to be processed
            ns3::Simulator::ScheduleNow(&PaxosAppServer::DoReceivedDecisionMessage, this, pktHeader);
        }
        else
        {
            NS_FATAL_ERROR("Unknown packet type");
            return; // Unknown packet type, ignore it
        }
    }
}

void
PaxosAppServer::DoReceivedProposalMessage(PaxosFrame frame)
{
    // Check if the proposer is the right one
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " receiving proposal message for Proposal ID " << frame.GetProposalId());
    uint32_t proposerId = frame.GetProposerId();

    // Set Accept time
    frame.SetAcceptorId(m_serverId);
    frame.SetAcceptTime(ns3::Simulator::Now());

    // Create a new proposal
    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
    proposal->setProposalId(frame.GetProposalId());
    proposal->setNodeId(frame.GetProposerId());
    proposal->setValue(frame.GetValue());
    proposal->setProposeTime(frame.GetProposeTime());
    proposal->setNumAck(0);
    proposal->setAcceptTime(ns3::Simulator::Now());

    // Add the proposal to the map
    m_acceptedProposals[frame.GetProposalId()] = proposal;

    ns3::Simulator::Schedule(ns3::NanoSeconds(10), &PaxosAppServer::SendAcceptMessage, this, frame);
}

void
PaxosAppServer::SendAcceptMessage(PaxosFrame frame)
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " sending accept message for Proposal ID " << frame.GetProposalId());

    frame.SetMessageType(PaxosFrame::ACCEPT);
    frame.SetAcceptorId(m_serverId);
    frame.SetAcceptTime(ns3::Simulator::Now());


    // Create Packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(frame);

    // Send the accept message to the proposer
    ns3::Ipv4Address address = m_nodes[frame.GetProposerId()].address;
    uint16_t port = m_nodes[frame.GetProposerId()].paxosPort;

    ns3::InetSocketAddress to(address, port);
    m_sendSocket->SendTo(packet, 0, to);
}

void
PaxosAppServer::DoReceivedAcceptMessage(PaxosFrame frame)
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " receiving accept message for Proposal ID " << frame.GetProposalId());

    uint64_t proposerId = frame.GetProposerId();
    uint64_t proposalId = frame.GetProposalId();
    uint32_t acceptorId = frame.GetAcceptorId();

    if (proposerId != m_serverId)
    {
        NS_LOG_WARN("PaxosAppServer " << m_nodeId << " ignoring accept message from wrong proposer ID " << proposerId);
        return; // Ignore the accept message if it is not from the right proposer
    }

    // Find the proposal in the map
    auto it = m_proposals.find(proposalId);
    if (it == m_proposals.end())
    {
        NS_LOG_WARN("PaxosAppServer " << m_nodeId << " ignoring accept message for unknown proposal ID " << proposalId);
        return; // Ignore the accept message if the proposal is not in the map
    }

    // Get the proposal
    std::shared_ptr<Proposal> proposal = it->second;
    // Increment the accept count
    proposal->incrementNumAck();

    // If the proposal has enough accepts, send a decision message
    if (proposal->getNumAck() >= (m_numNodes / 2))
    {
        // Add to our decided proposals queue
        NS_LOG_INFO("PaxosAppServer " << m_nodeId << " adding proposal ID " << proposalId << " to decided proposals queue.");
        proposal->setDecisionTime(ns3::Simulator::Now());

        m_decidedProposalQueue.push(proposal);

        SendDecisionMessage(frame);

        // Remove the proposal from the map
        m_proposals.erase(proposalId);
    }
    else
    {
        NS_LOG_INFO("PaxosAppServer " << m_nodeId << " proposal ID " << proposalId << " has not enough accepts yet.");
    }
}

void
PaxosAppServer::SendDecisionMessage(PaxosFrame frame)
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " sending decision message for Proposal ID " << frame.GetProposalId());
    // Create Header
    frame.SetMessageType(PaxosFrame::DECISION);
    frame.SetDecisionTime(ns3::Simulator::Now());

    // Create Packet
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(frame);

    // Send the decision message to all nodes
    for (auto node : m_nodes)
    {
        if (node.serverId == m_nodeId)
        {
            // Do not send the decision to itself
            continue;
        }

        ns3::Ipv4Address address = node.address;
        uint16_t port = node.paxosPort;

        // Send the packet to the node
        ns3::InetSocketAddress to(address, port);

        m_sendSocket->SendTo(packet, 0, to);
    }
}

void
PaxosAppServer::DoReceivedDecisionMessage(PaxosFrame frame)
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " received decision message for Proposal ID " << frame.GetProposalId());

    // Add the decision to the decided proposals queue
    std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
    proposal->setProposalId(frame.GetProposalId());
    proposal->setProposerId(frame.GetProposerId());
    proposal->setNodeId(frame.GetProposerId()); // Set the node ID to the current node
    proposal->setValue(frame.GetValue());
    proposal->setProposeTime(frame.GetProposeTime());
    proposal->setAcceptTime(frame.GetAcceptTime());
    proposal->setDecisionTime(frame.GetDecisionTime());

    // Add the proposal to the decided proposals queue
    m_decidedProposalQueue.push(proposal);

    // Since we don't need to add the proposal to the map, we don't need to delete it from the map
    // Delete the proposal from the proposals map
    
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " added proposal ID " << frame.GetProposalId() << " to decided proposals queue.");
}
