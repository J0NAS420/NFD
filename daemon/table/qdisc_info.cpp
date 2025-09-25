#include "qdisc_info.hpp"

namespace nfd {

uint32_t 
calculateIdleSlope (double bwfrac, uint32_t portTransmitRate)
{
    return std::ceil(bwfrac * portTransmitRate);
}

uint32_t 
calculateSendSlope (double bwfrac, uint32_t portTransmitRate)
{
    return std::floor(calculateIdleSlope(bwfrac, portTransmitRate) - portTransmitRate);
}

uint32_t 
calculateLoCredit (uint32_t maxFrameSize, double bwfrac, uint32_t portTransmitRate)
{
    return std::floor(maxFrameSize * ( (double) calculateSendSlope(bwfrac, portTransmitRate) / portTransmitRate ));
}

uint32_t 
calculateHiCredit (std::vector<srInfo> srInfoVector, genInfo genInfoStruct, std::vector<cbsConfigs> cbsConfigsVector, size_t entryIndex)
{
    double hicredit {0};
    int32_t index = entryIndex;
    while (index > 0) {
        int32_t prevIndex = index - 1;
        cbsConfigs prevCBSConfig = cbsConfigsVector.at(prevIndex);
        hicredit += ( prevCBSConfig.hiCredit / (-prevCBSConfig.sendSlope) ) + ( srInfoVector.at(prevIndex).maxFrameSize / genInfoStruct.portTransmitRate );
        --index;
    }
    hicredit += genInfoStruct.maxFrameSize / genInfoStruct.portTransmitRate;
    hicredit *= cbsConfigsVector.at(entryIndex).idleSlope;
    return std::ceil(hicredit);
}

std::vector<cbsConfigs> 
prepareCBSInfo (std::vector<srInfo> srInfoVector, genInfo genInfoStruct)
{
    std::vector<cbsConfigs> cbsConfigsVector = {};
    for (size_t i = 0; i < srInfoVector.size(); ++i) {
        double bwfrac = srInfoVector.at(i).assignedBitrate / genInfoStruct.portTransmitRate;
        
        cbsConfigs cbsConfigsStruct = {};
        cbsConfigsStruct.idleSlope = calculateIdleSlope(bwfrac, genInfoStruct.portTransmitRate);
        cbsConfigsStruct.sendSlope = calculateSendSlope(bwfrac, genInfoStruct.portTransmitRate);
        cbsConfigsStruct.loCredit = calculateLoCredit(srInfoVector.at(i).maxFrameSize, bwfrac, genInfoStruct.portTransmitRate);
        cbsConfigsStruct.hiCredit = calculateHiCredit(srInfoVector, genInfoStruct, cbsConfigsVector, i);

        cbsConfigsVector.push_back(cbsConfigsStruct);
    }
    return cbsConfigsVector;
}

} // namespace nfd