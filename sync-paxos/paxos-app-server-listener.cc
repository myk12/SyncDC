
#include "paxos-app-server.h"

NS_LOG_COMPONENT_DEFINE("PaxosAppServerListener");

void PaxosAppServer::StartListenerThread()
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " starting listener thread");
    // Create Listener UDP Socket
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_listenerSocket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (m_listenerSocket == nullptr)
    {
        NS_FATAL_ERROR("Failed to create socket");
    }

    // Get Server Listening Port
    ns3::Ipv4Address addr = ns3::Ipv4Address::GetAny();
    uint16_t port = m_nodes[m_nodeId].serverPort;
    ns3::Address local = ns3::InetSocketAddress(addr, port);
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " listening at " << addr << ":" << port);
    if (m_listenerSocket->Bind(local) == -1)
    {
        NS_FATAL_ERROR("Failed to bind socket");
    }
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " listening at " << local);

    // Set the receive callback
    m_listenerSocket->SetRecvCallback(MakeCallback(&PaxosAppServer::ReceiveRequest, this));
}

void
PaxosAppServer::StopListenerThread()
{
    NS_LOG_INFO("PaxosAppServer " << m_nodeId << " stopping listener thread");
    if (m_listenerSocket != nullptr)
    {
        m_listenerSocket->Close();
        m_listenerSocket = nullptr;
    }
}

void
PaxosAppServer::ReceiveRequest(ns3::Ptr<ns3::Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        NS_LOG_INFO("Received " << packet->GetSize() << " bytes from " << from);
        // TODO: Process the request

        RequestFrame requestFrame;
        packet->RemoveHeader(requestFrame);

        // Parse the request frame and push in the queue
        std::shared_ptr<Proposal> proposal = std::make_shared<Proposal>();
        // use the timestamp as proposal id
        ns3::Time timestamp = requestFrame.GetTimestamp();
        proposal->setProposalId(timestamp.GetNanoSeconds());
        proposal->setValue(requestFrame.GetValue());

        NS_LOG_INFO("Received proposal " << proposal->getProposalId() << " from " << from);

        // Add to queue
        m_waitingProposals.push(proposal);
    }
}
