#include "paxos-app-client.h"

NS_LOG_COMPONENT_DEFINE("PaxosAppClient");

PaxosAppClient::PaxosAppClient()
{
    NS_LOG_FUNCTION(this);
}

PaxosAppClient::PaxosAppClient(NodeInfoList nodes)
{
    NS_LOG_FUNCTION(this);
    m_servers = nodes;
}

PaxosAppClient::~PaxosAppClient()
{
    NS_LOG_FUNCTION(this);
}

ns3::TypeId
PaxosAppClient::GetTypeId(void)
{
    static ns3::TypeId tid = ns3::TypeId("PaxosAppClient")
        .SetParent<ns3::Application>()
        .AddConstructor<PaxosAppClient>();
        return tid;
}

void
PaxosAppClient::StartApplication(void)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Starting PaxosAppClient");

    // Create a UDP socket for sending messages
    auto tid = ns3::TypeId::LookupByName("ns3::UdpSocketFactory");
    m_socket = ns3::Socket::CreateSocket(GetNode(), tid);
    if (!m_socket)
    {
        NS_FATAL_ERROR("Failed to create UDP socket");
    }

    // Connect to each server
    for (auto server : m_servers)
    {
        // Connect to the server
        m_socket->Connect(server.address);
    }

    // Randomly Generate Reuqests and Send them to Servers
    m_sendRandom = ns3::CreateObject<ns3::UniformRandomVariable>();
    m_sendRandom->SetAttribute("Min", ns3::DoubleValue(100));
    m_sendRandom->SetAttribute("Max", ns3::DoubleValue(200));

    m_valueRandom = ns3::CreateObject<ns3::UniformRandomVariable>();
    m_valueRandom->SetAttribute("Min", ns3::DoubleValue(1));
    m_valueRandom->SetAttribute("Max", ns3::DoubleValue(1000000000)); // Random value between 1 and 100

    SendRequest();

}

void
PaxosAppClient::StopApplication(void)
{
    NS_LOG_FUNCTION(this);

    // Close socket and stop the application
    if (m_socket != nullptr)
    {
        m_socket->Close();
    }

    NS_LOG_INFO("Stopping PaxosAppClient");
}

void
PaxosAppClient::SendRequest()
{
    NS_LOG_FUNCTION(this);

    // Check if the application has already been stopped
    if (ns3::Simulator::Now() >= m_stopTime) {
        return;
    }

    // log time
    NS_LOG_INFO("Sending Request at " << ns3::Simulator::Now() << " seconds");
    // Generate Request and Send it to Servers
    std::shared_ptr<RequestFrame> request = std::make_shared<RequestFrame>();
    request->SetTimestamp(ns3::Simulator::Now());
    request->SetValue(m_valueRandom->GetInteger()); // Random value between 1 and 100

    // Create Packet
    NS_LOG_INFO("Request Frame created with Timestamp " << request->GetTimestamp() << ", Value " << request->GetValue()); 
    ns3::Ptr<ns3::Packet> packet = ns3::Create<ns3::Packet>();
    packet->AddHeader(*request);

    // Round Robin
    uint32_t curServerId = (m_lastServerId + 1) % m_servers.size();
    m_lastServerId = curServerId;
    NS_LOG_INFO("Sending Request to server " << curServerId << " at " << ns3::Simulator::Now() << " seconds");

    ns3::Ipv4Address address = m_servers[curServerId].address;
    uint16_t port = m_servers[curServerId].serverPort;

    NS_LOG_INFO("Sending Request to " << address << ":" << port << "");
    ns3::InetSocketAddress to(address, port);
    m_socket->SendTo(packet, 0, to);

    // Generate a random number between 1 and 10
    uint32_t interval = m_sendRandom->GetInteger();

    // Simulator::Schedule() takes a time and a function
    ns3::Simulator::Schedule(ns3::MicroSeconds(interval), &PaxosAppClient::SendRequest, this);

}
