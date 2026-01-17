//
// Created by chend on 2026/1/14.
//

#include "szMdParser.h"
#include "utility.h"


SZMDParser::SZMDParser() = default;

int SZMDParser::makeKey(const int marketCode, const int stockCode) {
    if (stockCode >= 1000000) return -1;
    int index = marketCode * 1000000 + stockCode;
    return index;
}

MyMDItem SZMDParser::makeElement(const int marketCode, const int stockCode) {
    MyMDItem mdItem = {marketCode, stockCode};
    return mdItem;
}


int SZMDParser::savePxNL(const int marketCode, const int stockCode, const int entryType, const int value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    auto it = myMDMap.find(index);
    if (it == myMDMap.end()) {
        MyMDItem mdItem = makeElement(marketCode, stockCode);
        mdItem.price[entryType] = value;
        myMDMap.insert({index,mdItem});
    } else {
        it->second.price[entryType] = value;
    }
    return 0;
}

int SZMDParser::loadPxNL(const int marketCode, const int stockCode, const int entryType, int &value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -2;
    auto it = myMDMap.find(index);
    if (it != myMDMap.end()) {
        return it->second.price[entryType];
    }
    return -1;
}

int SZMDParser::saveOrigTimeNL(const int marketCode,const int stockCode,const long long origTime) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    auto it = myMDMap.find(index);
    if (it == myMDMap.end()) {
        MyMDItem mdItem = makeElement(marketCode, stockCode);
        mdItem.origTime = origTime;
        mdItem.updateTime = getTimestampAsLongLong();
        myMDMap.insert({index,mdItem});
    } else {
        it->second.origTime = origTime;
        it->second.updateTime = getTimestampAsLongLong();
    }
    return 0;
}

int SZMDParser::saveMktStkCode(const int marketCode, const int stockCode) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    auto it = myMDMap.find(index);
    if (it == myMDMap.end()) {
        MyMDItem mdItem = makeElement(marketCode, stockCode);
        mdItem.marketCode = marketCode;
        mdItem.stockCode = stockCode;
        myMDMap.insert({index,mdItem});
    } else {
        it->second.marketCode = marketCode;
        it->second.stockCode = stockCode;
    }
    return 0;
}

int SZMDParser::parseNL(const RawSzMDData &mdData)  {
    constexpr int marketCode = 2;
    int stockCode = fixedCharToInt(mdData.SecurityID, sizeof(SecurityIDType));
    if (stockCode >= 1000000) return 1;
    if (saveMktStkCode(marketCode,stockCode) < 0) {

    }
    if (saveOrigTimeNL(marketCode,stockCode,mdData.OrigTime) < 0) {

    }
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
        if (savePxNL(marketCode, stockCode, charEntryType-'0', mdData.ExtendFields.MDEntryEntity[i].MDEntryPx) < 0) {

        }
    }
    return 0;
}

SZMDParser szMDParser;