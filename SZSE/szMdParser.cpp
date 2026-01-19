//
// Created by chend on 2026/1/14.
//

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "szMdParser.h"

#include <spdlog/fmt/bundled/base.h>

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


int SZMDParser::savePx(const int marketCode, const int stockCode, const int entryType, const int value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
        auto it = myMDMap.find(index);
        if (it == myMDMap.end()) {
            MyMDItem mdItem = makeElement(marketCode, stockCode);
            mdItem.price[entryType] = value;
            myMDMap.insert({index,mdItem});
        } else {
            it->second.price[entryType] = value;
        }
    }
    return 0;
}

int SZMDParser::loadPx(const int marketCode, const int stockCode, const int entryType, int &value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -2;
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
        auto it = myMDMap.find(index);
        if (it != myMDMap.end()) {
            return it->second.price[entryType];
        }
    }
    return -1;
}

int SZMDParser::saveOrigTime(const int marketCode,const int stockCode,const long long origTime) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
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
    }
    return 0;
}

int SZMDParser::saveMktStkCode(const int marketCode, const int stockCode) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
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
    }
    return 0;
}

int SZMDParser::parse(const RawSzMDData &mdData)  {
    constexpr int marketCode = 2;
    int stockCode = fixedCharToInt(mdData.SecurityID, sizeof(SecurityIDType));
    if (stockCode >= 1000000) return 1;
    if (saveMktStkCode(marketCode,stockCode) < 0) {

    }
    if (saveOrigTime(marketCode,stockCode,mdData.OrigTime) < 0) {

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
        if (savePx(marketCode, stockCode, charEntryType-'0', mdData.ExtendFields.MDEntryEntity[i].MDEntryPx) < 0) {

        }
    }
    return 0;
}

void SZMDParser::show() {
    int size = 0;
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
        size = myMDMap.size();
    }
    printf("myMDMap.size(): %d\n",size);
}

void SZMDParser::dump(const char* file_path) {
    printf("start dumping to %s...\n",file_path);
    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error opening file\n");
        close(fd);
    }
    {
        std::lock_guard<std::mutex> lock_guard(mtx);
        for (auto pair : myMDMap) {
            char lineBuf[256] = {};
            snprintf(lineBuf,256,"Key: %d\n", pair.first);
            size_t bytesWritten = write(fd, lineBuf, strlen(lineBuf));
            if (bytesWritten == -1) {
                perror("Error writing file\n");
                close(fd);
            }
            memset(lineBuf,0, 256);
            snprintf(lineBuf,256,"Value: %1d|%6d|%s|%lld|%lld|"
                                 "%d %d %d %d %d"
                                 "%d\n",
                pair.second.marketCode, pair.second.stockCode, pair.second.stockName, pair.second.origTime, pair.second.updateTime,
                pair.second.price[static_cast<size_t>(SzseEntry::BidPrice)],pair.second.price[static_cast<size_t>(SzseEntry::AskPrice)],pair.second.price[static_cast<size_t>(SzseEntry::LastPrice)],pair.second.price[static_cast<size_t>(SzseEntry::OpenPrice)],pair.second.price[static_cast<size_t>(SzseEntry::HighPrice)],
                pair.second.price[static_cast<size_t>(SzseEntry::LowPrice)]);
        }
    }
    printf("dump finished %s!\n",file_path);
    close(fd);
}

SZMDParser szMDParser;
