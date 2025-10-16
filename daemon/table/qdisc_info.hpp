#include <iostream>
#include <cstdint>
#include <cmath>
#include <vector>

struct srInfo {
  int32_t maxFrameSize;
  int32_t assignedBitrate;
};

struct genInfo {
  int32_t maxFrameSize;
  int32_t portTransmitRate;
};

struct cbsConfigs {
  int32_t idleSlope;
  int32_t sendSlope;
  int32_t hiCredit;
  int32_t loCredit;
};

std::vector<cbsConfigs> prepareCBSInfo (std::vector<srInfo> srInfoVector, genInfo genInfoStruct);
int32_t calculateIdleSlope (double bwfrac, int32_t portTransmitRate);
int32_t calculateSendSlope (double bwfrac, int32_t portTransmitRate);
int32_t calculateLoCredit (int32_t maxFrameSize, double bwfrac, int32_t portTransmitRate);
int32_t calculateHiCredit (std::vector<srInfo> srInfoVector, genInfo genInfoStruct, std::vector<cbsConfigs> cbsConfigsVector, size_t entryIndex);
