//
// Created by chend on 2026/1/8.
//

#ifndef SZSE_TYPES_H
#define SZSE_TYPES_H

#pragma pack(1)
//二进制解包 必须字节对齐
#define MAX_MD_ENTRY_NO 30
#include <cstdint>

struct ReqLogon {
    char szLocalName[20]{};
    char szTargetName[20]{};
    int iHeartBeat = 0;;
    char szPassword[16]{};
    char szVersion[32]{};
};


struct v5mdhead{
    uint32_t MsgType;
    uint32_t BodyLength;
};

struct v5mdtail{
    uint32_t Checksum;
};

using CompId = char[20];

struct v5mdLogonBody{
    CompId SenderCompID;
    CompId TargetCompID;
    int HeartBtInt;
    char Password[16];
    char DefaultApplVerID[32];
};

struct MsgLogon{
    v5mdhead head;
    v5mdLogonBody body;
    v5mdtail tail;
};

using NumInGroup = uint32_t;

using LocalTimeStamp = int64_t;
using SecurityIDType = char[8];
using Price = int64_t;
using Qty = int64_t;
using Amt = int64_t;

struct MDEntry{
    char MDEntryType[2];
    int64_t MDEntryPx;
    Qty MDEntrySize;
    uint16_t MDPriceLevel;
    int64_t NumberOfOrders;
    NumInGroup NoOrders;
    // Qty OrderQty;
};

//union ExtendFieldType {
struct ExtendFieldType {
    NumInGroup NoMDEntries{};
    MDEntry MDEntryEntity[MAX_MD_ENTRY_NO]{};
};
// };

struct RawSzMDData {
    LocalTimeStamp OrigTime;
    uint16_t ChannelNo;
    char MDStreamID[3];
    SecurityIDType SecurityID;
    char SecurityIDSource[4];
    char TradingPhaseCode[8];
    Price PrevClosePx;
    int64_t NumTrades;
    Qty TotalVolumeTrade;
    Amt TotalValueTrade;
    ExtendFieldType ExtendFields;
};

enum class SzseEntry {
    BidPrice = 0,    // 买入
    AskPrice = 1,    // 卖出
    LastPrice = 2,   // 最近价
    OpenPrice = 4,   // 开盘价
    HighPrice = 7,   // 最高价
    LowPrice = 8,    // 最低价
};

constexpr int NUM_FILED_SIZE = 10;

struct MyMDItem {
    int marketCode;
    int stockCode;
    char stockName[20];
    int price[NUM_FILED_SIZE];
    long long origTime;
    long long updateTime;
};

//cancel 1 byte pack
#pragma pack()

#endif //SZSE_TYPES_H