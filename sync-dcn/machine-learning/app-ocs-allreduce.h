#ifndef APP_OCS_ALLREDUCE
#define APP_OCS_ALLREDUCE

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#define OCS_ALL_REDUCE_PORT 9997
#define OCS_ALL_REDUCE_DATA_SIZE 2 * 1024 * 1024
#define OCS_ALL_REDUCE_PACKET_SIZE 1024
#define LOG_DIR "./"

#include <yaml-cpp/yaml.h>
#include <vector>
#include <fstream>

#include "ml-common.h"

class AppOCSAllReduce : public ns3::Application
{
public:
    static ns3::TypeId GetTypeId(void);
    AppOCSAllReduce();
    AppOCSAllReduce(uint32_t selfId,
                    std::vector<ns3::Ipv4Address> serversAddr,
                    std::string linkDelay, std::string linkBandwidth,
                    ns3::Time reConfTime, ns3::Time syncErrorTime,
                    std::map<ns3::Ipv4Address, uint32_t> MapAddr2Id,
                    uint64_t msgSize);
    ~AppOCSAllReduce();

    void StartApplication(void);
    void StopApplication(void);

    void StartOCSRecvThread(void);
    void StartOCSSendThread(void);

    // Recv functions
    void RecvDataCallback(ns3::Ptr<ns3::Socket> socket);
    bool RequestCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from);
    void AcceptCallback(ns3::Ptr<ns3::Socket> socket, const ns3::Address &from);
    void PeerCloseCallback(ns3::Ptr<ns3::Socket> socket);
    void PeerErrorCallback(ns3::Ptr<ns3::Socket> socket);

    // Send functions
    void SendToAllServers(void);
    void SendToServer(uint32_t targetId, int64_t sendSize);

private:
    uint32_t m_selfId;
    uint32_t m_serversNum;
    std::vector<ns3::Ipv4Address> m_serversAddr;
    ns3::Ptr<ns3::Socket> m_recvSocket;
    std::vector<ns3::Ptr<ns3::Socket>> m_recvSockets;

    std::map<uint32_t, uint64_t> m_recvBytes;
    uint32_t m_recvOKNum;

    std::map<ns3::Ipv4Address, uint32_t> m_MapAddr2Id;
    std::map<ns3::Ptr<ns3::Socket>, uint32_t> m_MapSocket2Id;

    std::map<uint32_t, ns3::Ptr<ns3::Socket>> m_sendSockets;
    std::map<uint32_t, ns3::Ptr<ns3::Application>> m_sendApps;
    std::map<uint32_t, uint64_t> m_sendBytes;
    uint64_t m_totalSendBytes;

    // Toplogy Info
    std::string m_linkDelay;
    std::string m_linkBandwidth;
    uint64_t m_linkDelayNanoSeconds;
    uint64_t m_linkBandwidthBps;
    ns3::Time m_reConfTime;
    ns3::Time m_syncErrorTime;
    ns3::Time m_sendSlot;
    ns3::Time m_OCSPeriod;
    int64_t m_sendSizePerSlot;
    uint64_t m_constMsgSize;     // in bytes

    std::fstream m_logFile;
};

#endif
