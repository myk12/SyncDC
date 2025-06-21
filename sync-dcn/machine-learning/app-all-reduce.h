#ifndef ML_APP_ALL_REDUCE_H
#define ML_APP_ALL_REDUCE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <fstream>

#define ALL_REDUCE_PORT 10000
#define MAX_PACKET_SIZE 1024
#define TARGET_BYTES (32 * 1024)

class AppAllReduce : public ns3::Application {
public: 
    static ns3::TypeId GetTypeId();
    AppAllReduce();
    ~AppAllReduce();
    void InitClusterInfo(uint32_t self_id,
                         const std::vector<ns3::Ipv4Address>& serverAddrs);
    void SetSendStartTime(ns3::Time startTime);

    void StartApplication();
    void StopApplication();

private:
    void HandleRead(ns3::Ptr<ns3::Socket> socket);
    bool HandleRequest(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
    void HandleAccept(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
    void HandlePeerClose(ns3::Ptr<ns3::Socket> socket);
    void HandlePeerError(ns3::Ptr<ns3::Socket> socket);

    uint32_t GetServerIdByAddress(const ns3::Ipv4Address& addr);

    uint32_t m_selfId;
    uint64_t m_targetBytes;
    uint32_t m_okServerNum;

    ns3::Time m_sendStartTime;

    std::string m_logfilename;

    ns3::Ipv4Address m_selfAddress;
    ns3::Ptr<ns3::Socket> m_listenSocket; // TCP
    std::map<uint32_t, ns3::Ptr<ns3::Socket>> m_recvSockets; // TCP
    std::map<uint32_t, uint64_t> m_id2RecvBytes; // TCP
    uint32_t m_recvOverNum;

    std::map<uint32_t, ns3::Ipv4Address> m_id2ServerAddr; // TCP

    ns3::ApplicationContainer m_bulkSendApps;
};

#endif
