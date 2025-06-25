#ifndef APP_FASTPASS_ARBITER_H
#define APP_FASTPASS_ARBITER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"

#include "header-fastpass.h"
#include "spine-leaf.h"
#include "ml-common.h"

#define FASTPASS_ARBITER_PORT 9995

class FastPassArbiter : public ns3::Application
{
public:
    static ns3::TypeId GetTypeId();
    FastPassArbiter();
    ~FastPassArbiter() override;

    void StartApplication() override;
    void StopApplication() override;

    void InitListener();
    void InitResourcePool();

    // Callback functions
    bool HandleAccept(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
    void HandleAcceptError(ns3::Ptr<ns3::Socket> socket, const ns3::Address& from);
    void HandleRead(ns3::Ptr<ns3::Socket> socket);

    // Process Request
    void ProcessRequest(ns3::Ptr<ns3::Socket> socket, FastPassHeader header);
    ns3::Time AllocateIntraLeafResource(uint32_t leafId, uint64_t dataSize);
    ns3::Time AllocateInterLeafResource(uint32_t srcLeafId, uint32_t dstLeafId, uint64_t dataSize);
    ns3::Time DataSize2TimeSlot(uint64_t dataSize);
    // Response
    void SendResponse(ns3::Ptr<ns3::Socket> socket, ns3::Ptr<ns3::Packet> packet);

private:
    // resource pool
    // node Id to position
    ns3::Ptr<ns3::Socket> m_listenSocket;
    std::shared_ptr<SyncDCTopologySpineLeaf> m_topology;

    // Define the resource pool
    // key: leaf Id
    // value: queues of next available time
    // Note: for each leaf, we have number of queues equal to number of spines
    //       each queue is the next available time for each spine
    std::map<uint32_t, std::vector<ns3::Time>> m_crossLeafResourcePool;
    std::map<uint32_t, std::vector<ns3::Time>> m_inLeafResourcePool;
};

#endif // APP_FASTPASS_ARBITER_H
