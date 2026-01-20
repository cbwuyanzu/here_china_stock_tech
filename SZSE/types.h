//
// Created by chend on 2026/1/8.
//

#ifndef SZSE_TYPES_H
#define SZSE_TYPES_H

#include <cstdint>
#include <unordered_map>

#pragma pack(1)
//二进制解包 必须字节对齐
#define MAX_MD_ENTRY_NO 30

using CompId = char[20];
using NumInGroup = uint32_t;
using LocalTimeStamp = int64_t;
using SecurityIDType = char[8];
using Price = int64_t;
using Qty = int64_t;
using Amt = int64_t;

struct ReqLogonCfg {
    char szLocalName[20];
    char szTargetName[20];
    int iHeartBeat = 0;
    char szPassword[16];
    char szVersion[32];
};

struct v5MDHead{
    uint32_t MsgType;
    uint32_t BodyLength;
};

struct v5MDTail{
    uint32_t Checksum;
};

struct v5mdLogonBody{
    CompId SenderCompID;
    CompId TargetCompID;
    int HeartBtInt;
    char Password[16];
    char DefaultApplVerID[32];
};

struct MsgReqLogon{
    v5MDHead head;
    v5mdLogonBody body;
    v5MDTail tail;
};

struct MDEntry{
    char MDEntryType[2];
    int64_t MDEntryPx;
    Qty MDEntrySize;
    uint16_t MDPriceLevel;
    int64_t NumberOfOrders;
    NumInGroup NoOrders;
    // Qty OrderQty;
};

struct ExtendFieldType {
    NumInGroup NoMDEntries;
    MDEntry MDEntryEntity[MAX_MD_ENTRY_NO];
};

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

union v5RecvMsgBody {
    RawSzMDData r300111;
    char charArray[4096];
};

struct v5QueueData {
    v5MDHead parsedHead;
    v5RecvMsgBody notParsedBody;
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

struct funcStat {
    long long success;
    long long fail;
    long long successTimeCostMs;
    long long failTimeCostMs;
};
using FuncStatMap = std::unordered_map<uint32_t, funcStat>;
#endif //SZSE_TYPES_H