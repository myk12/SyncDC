#ifndef _PAXOS_APP_CLIENT_H_
#define _PAXOS_APP_CLIENT_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application.h"

#include "ns3/random-variable-stream-helper.h"

#include "paxos-common.h"
#include "paxos-frame.h"

// The PaxosAppClient class implements a client application that sends
// requests to the PaxosApp server.

// The client reads requests from a file and sends them to the server.

class PaxosAppClient : public ns3::Application
{
public:
    PaxosAppClient();
    PaxosAppClient(NodeInfoList nodes);
    ~PaxosAppClient();

    static ns3::TypeId GetTypeId(void);

    void StartApplication(void) override;
    void StopApplication(void) override;

    void SetSendInterval(ns3::Time interval);

private:
    void SendRequest();
    uint32_t m_lastServerId; // ID of the last server that the client sent a request to 
    NodeInfoList m_servers; // List of all servers in the network
    ns3::Time m_sendInterval; // Interval between sending requests  
    ns3::Ptr<ns3::Socket> m_socket;
    ns3::Ptr<ns3::RandomVariableStream> m_sendRandom; // Random variable for request intervals
    ns3::Ptr<ns3::RandomVariableStream> m_valueRandom; // Random variable for request values
};

#endif // _PAXOS_APP_CLIENT_H_
