#include "ml-common.h"

NS_LOG_COMPONENT_DEFINE("MLCommon");

// Bandwidth Sting format: 100Gbps, 10Gbps, 1Gbps, 100Mbps, 10Mbps, 1Mbps
uint64_t
BandwidthStr2Bps(std::string bandwidthStr)
{
    // Get number
    std::string numberStr = bandwidthStr.substr(bandwidthStr.find("bps") - 1);
    uint64_t number = std::stoull(numberStr);

    // Get unit
    std::string unitStr = bandwidthStr.substr(bandwidthStr.find("bps") + 3);
    uint64_t unit = 1;
    if (unitStr == "Gbps") {
        unit = 1000000000;
    } else if (unitStr == "Mbps") {
        unit = 1000000;
    } else if (unitStr == "Kbps") {
        unit = 1000;
    } else if (unitStr == "bps") {
        unit = 1;
    } else {
        NS_LOG_ERROR("Unsupport bandwidth unit: " << unitStr);
    }

    // Note we are returning Bytes not bits
    return number * unit / 8;
}

// Delay String format: 100ms, 10ms, 1ms, 100us, 10us, 1us, 100ns, 10ns, 1ns
uint64_t
DelayStr2NanoSeconds(std::string delayStr)
{
    // Get number
    std::string unitPart = delayStr.substr(delayStr.size() - 2);
    // Get unit
    std::string numberPart = delayStr.substr(0, delayStr.size() - 2);

    uint64_t number = std::stoull(numberPart);
    uint64_t unit = 1;
    if (unitPart == "ms") {
        unit = 1000000;
    } else if (unitPart == "us") {
        unit = 1000;
    } else if (unitPart == "ns") {
        unit = 1;
    } else {
        NS_FATAL_ERROR("Unsupport delay unit: " << unitPart);
    }

    return number * unit;
}