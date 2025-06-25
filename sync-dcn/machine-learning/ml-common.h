#ifndef ML_COMMON_H
#define ML_COMMON_H

#include "ns3/core-module.h"

#include <string>
#include <vector>

enum DCN_TYPE {
    DCN_CLOS,
    DCN_OCS,
    DCN_FASTPASS
};

uint64_t BandwidthStr2Bps(std::string bandwidth);
uint64_t DelayStr2NanoSeconds(std::string delay);
uint64_t DataSizeStr2Bytes(std::string dataSize);

#endif
