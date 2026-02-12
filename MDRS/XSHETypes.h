//
// Created by chend on 2026/1/8.
//

#ifndef SZSE_TYPES_H
#define SZSE_TYPES_H

#include <cstdint>
#include <unordered_map>

#pragma pack(1)
//二进制解包 必须字节对齐
constexpr int MAX_MD_ENTRY_NO = 30;
// constexpr int MAX_MD_ENTRY_NO 30

using CompId = char[20];
using SessionStatus = int32_t;
using Text = char[200];


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


struct v5mdLogoutBody {
    SessionStatus session_status;
    Text text;
};

struct MsgReqLogout{
    v5MDHead head;
    v5mdLogoutBody body;
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

struct HkMDEntry{
    char MDEntryType[2];
    int64_t MDEntryPx;
    Qty MDEntrySize;
    uint16_t MDPriceLevel;
};

struct HkVcm{
    NumInGroup NoComplexEventTimes;
    LocalTimeStamp ComplexEventStartTime;
    LocalTimeStamp ComplexEventEndTime;
};

struct HKExtendFieldType {
    NumInGroup NoMDEntries;
    HkMDEntry MDEntryEntity[MAX_MD_ENTRY_NO];
    HkVcm hkVcm;
};

struct RawSzHkMDData {
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
    HKExtendFieldType ExtendFields;
};

struct RawSzHkMarketStatus {
    LocalTimeStamp OrigTime;
    uint16_t ChannelNo;
    char MarketId[8];
    char MarketSegmentID[8];//Reserve
    char TradingSessionID[4];
    char TradingSessionSubID[4];
    uint16_t TradSesStatus;//Reserve
    LocalTimeStamp TradSesStartTime;//Reserve
    LocalTimeStamp TradSesEndTime;//Reserve
    Amt ThresholdAmount;
    Amt PosAmt;
    char AmountStatus;//1-额度不可用 2-额度可用 3-额度充足
};

struct ChannelStatEntry{
    char MDStreamID[3];
    uint32_t StockNum;
    char TradingPhaseCode[8];
};

struct RawSzChannelStat {
    LocalTimeStamp OrigTime;
    uint16_t ChannelNo;
    NumInGroup NoMDSteamId;
    ChannelStatEntry ChannelStatEntity[MAX_MD_ENTRY_NO];
};

union v5RecvMsgBody {
    RawSzMDData r300111;
    RawSzHkMarketStatus r390019;
    RawSzChannelStat r390090;
    RawSzHkMDData r306311;
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
    long long successTimeCostUs;
    long long failTimeCostUs;
};
using FuncStatMap = std::unordered_map<uint32_t, funcStat>;


const std::unordered_map<int,const char*> DictTradingSessionSubID = {
    {-1,   "INIT"},
    {0 , "全日收市"},
    {1 , "输入买卖盘(开盘集合竞价时段)"},
    {2 , "对盘(开盘集合竞价时段)"},
    {3 , "持续交易"},
    {4 , "对盘(收盘集合竞价时段)"},
    {5 , "输入买卖盘(收盘集合竞价时段)"},
    {7 , "暂停"},
    {100 , "未开市"},
    {101 , "对盘前(开盘集合竞价时段)"},
    {102 , "Exchange Intervention"},
    {103 , "收市"},
    {104 , "取消买卖盘"},
    {105 , "参考价定价(收盘集合竞价时段)"},
    {106 , "不可取消(收盘集合竞价时段)"},
    {107 , "随机收市(收盘集合竞价时段)"},
    {108 , "随机对盘(开盘集合竞价时段)"}
};
#endif //SZSE_TYPES_H