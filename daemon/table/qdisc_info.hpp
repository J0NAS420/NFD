#include <iostream>
#include <cstdint>
#include <cmath>
#include <vector>

struct srInfo {
  uint32_t maxFrameSize;
  uint32_t assignedBitrate;
};

struct genInfo {
  uint32_t maxFrameSize;
  uint32_t portTransmitRate;
};

struct cbsConfigs {
    uint32_t idleSlope;
    uint32_t sendSlope;
    uint32_t hiCredit;
    uint32_t loCredit;
};

std::vector<cbsConfigs> prepareCBSInfo (std::vector<srInfo> srInfoVector, genInfo genInfoStruct);
int32_t calculateIdleSlope (double bwfrac, int32_t portTransmitRate);
int32_t calculateSendSlope (double bwfrac, int32_t portTransmitRate);
int32_t calculateLoCredit (int32_t maxFrameSize, double bwfrac, int32_t portTransmitRate);
int32_t calculateHiCredit (std::vector<srInfo> srInfoVector, genInfo genInfoStruct, std::vector<cbsConfigs> cbsConfigsVector, size_t entryIndex);
