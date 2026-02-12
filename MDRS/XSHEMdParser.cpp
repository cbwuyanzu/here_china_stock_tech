//
// Created by chend on 2026/1/14.
//

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include "XSHEMdParser.h"
#include <spdlog/fmt/bundled/base.h>
#include "myPub/utility.h"


MDParser::MDParser() : myMDmap("MDParser.shm") {
}

int MDParser::makeKey(const int marketCode, const int stockCode) {
    if (stockCode >= 1000000) return -1;
    int index = marketCode * 1000000 + stockCode;
    return index;
}

MyMDItem MDParser::makeElement(const int marketCode, const int stockCode) {
    MyMDItem mdItem = {marketCode, stockCode};
    return mdItem;
}

MyMDItem* MDParser::getElement(int marketCode, int stockCode) {
    auto key = makeKey(marketCode, stockCode);
    if (key == -1) {
        return nullptr;
    }
    return myMDmap.getPtr(key);
}


int MDParser::savePx(const int marketCode, const int stockCode, const int entryType, const int value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -2;
    {
        auto it = getElement(marketCode, stockCode);
        if (it == nullptr) {
            MyMDItem mdItem = makeElement(marketCode, stockCode);
            mdItem.price[entryType] = value;
            myMDmap.set(index,mdItem);
        } else {
            it->price[entryType] = value;
        }
    }
    return 0;
}

int MDParser::loadPx(const int marketCode, const int stockCode, const int entryType, int &value) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -2;
    {
        auto it = getElement(marketCode, stockCode);
        if (it != nullptr) {
            return it->price[entryType];
        }
    }
    return -1;
}

int MDParser::saveOrigTime(const int marketCode,const int stockCode,const long long origTime) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    {
        auto it = getElement(marketCode, stockCode);
        if (it == nullptr) {
            MyMDItem mdItem = makeElement(marketCode, stockCode);
            mdItem.origTime = origTime;
            mdItem.updateTime = getTimestampAsLongLong();
            myMDmap.set(index,mdItem);
        } else {
            it->origTime = origTime;
            it->updateTime = getTimestampAsLongLong();
        }
    }
    return 0;
}

int MDParser::saveMktStkCode(const int marketCode, const int stockCode) {
    int index = makeKey(marketCode, stockCode);
    if (index < 0) return -1;
    {
        auto it = getElement(marketCode, stockCode);
        if (it == nullptr) {
            MyMDItem mdItem = makeElement(marketCode, stockCode);
            mdItem.marketCode = marketCode;
            mdItem.stockCode = stockCode;
            myMDmap.set(index,mdItem);
        } else {
            it->marketCode = marketCode;
            it->stockCode = stockCode;
        }
    }
    return 0;
}

int MDParser::parse(const RawSzMDData &mdData)  {
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

int MDParser::parse(const RawSzHkMDData &mdData)  {
    constexpr int marketCode = 9;
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

void MDParser::show() {
    size_t size = 0;
    {
        size = myMDmap.size();
    }
    printf("myMDMap.size(): %lu\n",size);
}

void MDParser::dump(const char* file_path) {
    printf("start dumping to %s...\n",file_path);
    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error opening file\n");
        close(fd);
        return;
    }
    int i = 0;
    long long currentTimeWithMs = getTimestampAsLongLong();
    char lineBuf[256] = {};
    snprintf(lineBuf, sizeof(lineBuf), "%lld\n", currentTimeWithMs);
    write(fd,lineBuf,strlen(lineBuf));
    {
        myMDmap.forEach([&](MyMDItem& record) {
            ++i;
            memset(lineBuf,0, 256);
            snprintf(lineBuf,256,"%d\n", makeKey(record.marketCode, record.stockCode));
            size_t bytesWritten = write(fd, lineBuf, strlen(lineBuf));
            if (bytesWritten == -1) {
                perror("Error writing file\n");
                close(fd);
                return;
            }
            memset(lineBuf,0, 256);
            snprintf(lineBuf,256,"%1d|%06d|%s|%lld|%lld|"
                                 "%9d %9d %9d %9d %9d "
                                 "%9d\n",
                record.marketCode, record.stockCode, record.stockName, record.origTime, record.updateTime,
                record.price[static_cast<size_t>(SzseEntry::BidPrice)],record.price[static_cast<size_t>(SzseEntry::AskPrice)],record.price[static_cast<size_t>(SzseEntry::LastPrice)],record.price[static_cast<size_t>(SzseEntry::OpenPrice)],record.price[static_cast<size_t>(SzseEntry::HighPrice)],
                record.price[static_cast<size_t>(SzseEntry::LowPrice)]);
            bytesWritten = write(fd, lineBuf, strlen(lineBuf));
            if (bytesWritten == -1) {
                perror("Error writing file\n");
                close(fd);
                return;
            }
        });

    }
    printf("dump finished %s %d Lines!\n\n",file_path, i);
    close(fd);
}

void MDParser::clear() {
    myMDmap.clear();
}


MDParser gMDParser;
