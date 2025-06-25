#ifndef HEADER_FASTPASS_H
#define HEADER_FASTPASS_H

#include "ns3/header.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

class FastPassHeader : public ns3::Header
{
public:
    static ns3::TypeId GetTypeId();
    ns3::TypeId GetInstanceTypeId() const override;
    FastPassHeader();
    ~FastPassHeader() override;

    // serialize and deserialize
    void Serialize(ns3::Buffer::Iterator start) const override;
    uint32_t Deserialize(ns3::Buffer::Iterator start) override;
    uint32_t GetSerializedSize() const override;
    void Print(std::ostream& os) const override;

    void SetSrcId(uint32_t srcId);
    uint32_t GetSrcId() const;
    void SetDstId(uint32_t dstId);
    uint32_t GetDstId() const;

    void SetDataSize(uint32_t dataSize);
    uint32_t GetDataSize() const;

    void SetSuccess(uint32_t success);
    uint32_t GetSuccess() const;

    void SetStartTime(ns3::Time startTime);
    ns3::Time GetStartTime() const;

private:
    // Request frame fields
    uint32_t m_srcId;   // Source ID
    uint32_t m_dstId;   // Destination ID
    uint64_t m_dataSize; // Data size

    // Response frame fields
    uint32_t m_success; // 0: fail, 1: success
    ns3::Time m_startTime; // Allocated time that can send data
};

#endif // HEADER_FASTPASS_H
