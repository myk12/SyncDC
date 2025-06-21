#ifndef APP_RING_ALLREDUCE_H
#define APP_RING_ALLREDUCE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <yaml-cpp/yaml.h>
#include <vector>

#define RING_ALL_REDUCE_PORT 9999
#define RING_ALL_REDUCE_DATA_SIZE 32*1024*1024

class AppRingAllReduce : public ns3::Application {
    public:
        static ns3::TypeId GetTypeId();
        AppRingAllReduce();
        AppRingAllReduce(uint32_t selfId, std::vector<ns3::Ipv4Address> serversAddr);
        ~AppRingAllReduce();

        void StartApplication();
        void StopApplication();

        void StartRingRecvThread();
        void StartRingSendThread();

        // Recv functions
        void RecvDataCallback(ns3::Ptr<ns3::Socket> socket);
        bool RequestCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
        void AcceptCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
        void PeerCloseCallback(ns3::Ptr<ns3::Socket> socket);
        void PeerErrorCallback(ns3::Ptr<ns3::Socket> socket);

        // Send functions
        void StartBulkSendInstance(uint32_t sendRound);
    private:
        uint32_t m_selfId;
        uint32_t m_prevId;
        uint32_t m_nextId;
        uint32_t m_serversNum;
        std::vector<ns3::Ipv4Address> m_serversAddr;
        ns3::Ptr<ns3::Socket> m_recvSocket;
        ns3::Ptr<ns3::Socket> m_sendSocket;
        std::vector<ns3::Ptr<ns3::Socket>> m_recvSockets;

        uint32_t m_recvRound;
        uint64_t m_recvBytes;

        uint32_t m_sendRound;
        ns3::InetSocketAddress m_sendAddr;
        std::map<uint32_t, uint64_t> m_sendBytes;
        std::map<uint32_t, ns3::Ptr<ns3::Application>> m_sendApps;
};

#endif
