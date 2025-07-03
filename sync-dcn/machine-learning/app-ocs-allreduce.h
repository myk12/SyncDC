#ifndef APP_OCS_ALLREDUCE
#define APP_OCS_ALLREDUCE

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/timestamp-tag.h"

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
                    ns3::Ipv4Address targetAddr,
                    ns3::Ipv4Address bindAddr,
                    ns3::Ipv4Address listenAddr,
                    uint64_t constMsgSize,
                    std::string logDir);
    ~AppOCSAllReduce();

    void StartApplication(void);
    void StopApplication(void);

    void StartOCSRecvThread(void);
    void StartOCSSendThread(void);

    // Recv functions
    void RecvDataCallback(ns3::Ptr<ns3::Socket> socket);

    // Send functions
    void SendData();

private:
    uint32_t m_selfId;
    ns3::Ipv4Address m_targetAddr;
    ns3::Ipv4Address m_bindAddr;
    ns3::Ipv4Address m_listenAddr;
    uint64_t m_constMsgSize;     // in bytes
    std::string m_logDir;
    ns3::InetSocketAddress m_target;

    ns3::Ptr<ns3::Socket> m_recvSocket;
    uint64_t m_recvBytes;

    ns3::Ptr<ns3::Socket> m_sendSocket;
    uint64_t m_sendBytes;

    std::string m_logFileName;
    std::fstream m_logFile;
};

#endif
