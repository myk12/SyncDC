#include "ml-common.h"

NS_LOG_COMPONENT_DEFINE("MLCommon");

// Bandwidth Sting format: 100Gbps, 10Gbps, 1Gbps
uint64_t
BandwidthStr2Bps(std::string bandwidthStr)
{
    // Get number
    std::string numStr = bandwidthStr.substr(0, bandwidthStr.length() - 4);

    uint64_t number = std::stoull(numStr);
    NS_LOG_INFO("Bandwidth number: " << number);
    uint64_t unit = 1; // Default is bps
    if (bandwidthStr.find("Gbps") != std::string::npos) {
        NS_LOG_INFO("Bandwidth unit: Gbps");
        unit = 1000000000; // 1 Gbps = 10^9
    } else if (bandwidthStr.find("Mbps") != std::string::npos) {
        unit = 1000000; // 1 Mbps = 10^6
    } else if (bandwidthStr.find("Kbps") != std::string::npos) {
        unit = 1000; // 1 Kbps = 10^3
    } else {
        NS_FATAL_ERROR("Unsupport bandwidth unit: " << bandwidthStr);
    }
    return number * unit / 8; // Convert to bytes per second
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

uint64_t
DataSizeStr2Bytes(std::string dataSizeStr)
{
    // Get number
    std::string numStr = dataSizeStr.substr(0, dataSizeStr.length() - 2);

    uint64_t number = std::stoull(numStr);
    NS_LOG_INFO("Data size number: " << number);
    uint64_t unit = 1; // Default is byte
    if (dataSizeStr.find("KB") != std::string::npos) {
        NS_LOG_INFO("Data size unit: KB");
        unit = 1024;
    } else if (dataSizeStr.find("MB") != std::string::npos) {
        unit = 1024 * 1024;
    } else if (dataSizeStr.find("GB") != std::string::npos) {
        unit = 1024 * 1024 * 1024;
    } else {
        NS_FATAL_ERROR("Unsupport data size unit: " << dataSizeStr);
    }
    return number * unit;
}
