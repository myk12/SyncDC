#ifndef APP_RING_ALLREDUCE_H
#define APP_RING_ALLREDUCE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

#include <vector>
#include <fstream>

#define LOG_DIR "./"

class AppRingAllReduce : public ns3::Application {
    public:
        static ns3::TypeId GetTypeId();
        AppRingAllReduce();
        AppRingAllReduce(uint32_t selfIdx, // index in this ring 
                        std::vector<std::pair<uint32_t, ns3::Ipv4Address>> serverIDAddrs,  // 
                        uint16_t port,  // service port
                        uint64_t msgSize,
                        uint32_t jobID,
                        std::string logDir);
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
        uint32_t m_selfNodeId;
        uint32_t m_selfIdx;
        uint32_t m_prevIdx;
        uint32_t m_nextIdx;
        uint32_t m_serversNum;
        uint16_t m_servicePort;
        std::vector<std::pair<uint32_t, ns3::Ipv4Address>> m_serverIDAddrs;
        ns3::Ptr<ns3::Socket> m_recvSocket;
        ns3::Ptr<ns3::Socket> m_sendSocket;
        std::vector<ns3::Ptr<ns3::Socket>> m_recvSockets;

        uint32_t m_recvRound;
        uint64_t m_recvBytes;

        uint32_t m_sendRound;
        ns3::InetSocketAddress m_sendAddr;
        std::map<uint32_t, uint64_t> m_sendBytes;
        std::map<uint32_t, ns3::Ptr<ns3::Application>> m_sendApps;

        uint64_t m_msgSize; // Size of the message to send in bytes

        // Log file
        uint32_t m_jobID;
        std::ofstream m_logfile;
        std::string m_logPath;
        std::string m_delayLogPath;
        std::ofstream m_delayLogfile;
};

#endif
