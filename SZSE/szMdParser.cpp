//
// Created by chend on 2026/1/14.
//

#include "szMdParser.h"
#include "utility.h"


SZMDParser::SZMDParser() {
    myMDDataList.reserve(10 * 000000);
}

int SZMDParser::savePxNL(const int marketCode, const int stockCode, const int entryType, const int value) {
    if (stockCode >= 1000000) return 1;
    int index = marketCode * 1000000 + stockCode;
    myMDDataList[index].price[entryType] = value;
    return 0;
}


int SZMDParser::loadPxNL(const int marketCode, const int stockCode, const int entryType, int &value) {
    if (stockCode >= 1000000) return 1;
    int index = marketCode * 1000000 + stockCode;
    value = myMDDataList[index].price[entryType];
    return 0;
}

int SZMDParser::saveOrigTimeNL(const int marketCode,const int stockCode,const long long origTime) {
    if (stockCode >= 1000000) return 1;
    int index = marketCode * 1000000 + stockCode;
    myMDDataList[index].origTime = origTime;
    myMDDataList[index].updateTime = getTimestampAsLongLong();
    return 0;
}

int SZMDParser::saveMktStkCode(const int marketCode, const int stockCode) {
    if (stockCode >= 1000000) return 1;
    int index = marketCode * 1000000 + stockCode;
    myMDDataList[index].marketCode = marketCode;
    myMDDataList[index].stockCode = stockCode;
    return 0;
}

int SZMDParser::parseNL(const RawSzMDData &mdData)  {
    constexpr int marketCode = 2;
    int stockCode = fixedCharToInt(mdData.SecurityID, sizeof(SecurityIDType));
    if (stockCode >= 1000000) return 1;
    saveMktStkCode(marketCode,stockCode);
    saveOrigTimeNL(marketCode,stockCode,mdData.OrigTime);
    for (int i = 0; i < mdData.ExtendFields.NoMDEntries; i++) {
        char charEntryType = mdData.ExtendFields.MDEntryEntity[i].MDEntryType[0];
        if (charEntryType < '0' || charEntryType > '9' ) {
            //x?这类暂时不支持
            continue;
        }
        if (mdData.ExtendFields.MDEntryEntity[i].MDPriceLevel > 1){
            //买卖2-5暂时不支持
            continue;
        }
        savePxNL(marketCode, stockCode, charEntryType-'0', mdData.ExtendFields.MDEntryEntity[i].MDEntryPx);
    }
    return 0;
}

SZMDParser szMDParser;