//
// Created by chend on 2026/1/19.
//

#include "business.h"
#include "myFIFOQueue.h"
#include "loggerSingleton.h"
#include "utility.h"
#include "szMdParser.h"

extern  SZMDParser szMDParser;
extern MyFifoQueue<v5QueueData> queueForSZMarketData;

void popAndParse(int timeoutMs){
    v5QueueData queueData{};
    queueForSZMarketData.try_pop(queueData,timeoutMs);
    switch (queueData.parsedHead.MsgType) {
        case 300111:
            RawSzMDData md = {};
            if(deserialize300111(md,queueData.notParsedBody.charArray,queueData.parsedHead.BodyLength)) {
                LOG_ERROR("deserialize body failed");
                break;
            }
            OnRealTimeMD(md);
            break;
        // default: ;
    }
}

void OnRealTimeMD(const RawSzMDData &md) {
    static int rtmdcount = 0;
    ++rtmdcount;
    if (rtmdcount % 100 == 0)
        LOG_INFO("RealTimeMDCount\t{}",rtmdcount);
    szMDParser.parse(md);
    showMdData(md);
}

int deserialize300111(RawSzMDData &md,const void* buffer, uint32_t length) {
    auto *p = static_cast<const RawSzMDData*>(buffer);
    //这里先做一些包的检查, 以免引起coredump
    uint32_t uNoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    if ( uNoMDEntries > MAX_MD_ENTRY_NO) {
        return -1;
    }
    if (length != uNoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup) + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md)) {
        LOG_ERROR("NoMDEntries:\t{}\nlength:\t{}\nExpect length:{}",
            md.ExtendFields.NoMDEntries, length,
            md.ExtendFields.NoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup)
            + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md));
        return -2;
    }
    memset(&md, 0, sizeof(md));
    memcpy(&md, buffer, length);
    md.OrigTime = htn64(p->OrigTime);
    md.ChannelNo = htnu16(p->ChannelNo);
    md.PrevClosePx = htn64(p->PrevClosePx);
    md.NumTrades = htn64(p->NumTrades);
    md.TotalVolumeTrade = htn64(p->TotalVolumeTrade);
    md.TotalValueTrade = htn64(p->TotalValueTrade);
    md.ExtendFields.NoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    for (int i = 0; i < md.ExtendFields.NoMDEntries; i++) {
        md.ExtendFields.MDEntryEntity[i].MDEntryPx = htn64(p->ExtendFields.MDEntryEntity[i].MDEntryPx);
        md.ExtendFields.MDEntryEntity[i].MDEntrySize = htn64(p->ExtendFields.MDEntryEntity[i].MDEntrySize);
        md.ExtendFields.MDEntryEntity[i].MDPriceLevel = htnu16(p->ExtendFields.MDEntryEntity[i].MDPriceLevel);
        md.ExtendFields.MDEntryEntity[i].NumberOfOrders = htn64(p->ExtendFields.MDEntryEntity[i].NumberOfOrders);
        md.ExtendFields.MDEntryEntity[i].NoOrders = htnu32(p->ExtendFields.MDEntryEntity[i].NoOrders);
    }
    return 0;
}

void showMdData(const RawSzMDData & md) {
    LOG_DEBUG("OrigTime:{}",md.OrigTime);
    LOG_DEBUG("ChannelNo:{}",md.ChannelNo);
    LOG_DEBUG("MDStreamID:{:.{}}",md.MDStreamID, sizeof(md.MDStreamID));
    LOG_DEBUG("SecurityID:{:.{}}",md.SecurityID, sizeof(md.SecurityID));
    LOG_DEBUG("SecurityIDSource:{:.{}}",md.SecurityIDSource, sizeof(md.SecurityIDSource));
    LOG_DEBUG("TradingPhaseCode:{:.{}}",md.TradingPhaseCode, sizeof(md.TradingPhaseCode));
    LOG_DEBUG("PrevClosePx:{}",md.PrevClosePx);
    LOG_DEBUG("NumTrades:{}",md.NumTrades);
    LOG_DEBUG("TotalVolumeTrade:{}",md.TotalVolumeTrade);
    LOG_DEBUG("TotalValueTrade:{}",md.TotalValueTrade);
    LOG_DEBUG("ExtendFields:");
    LOG_DEBUG("NoMDEntries:{}",md.ExtendFields.NoMDEntries)
    for (int i = 0; i < md.ExtendFields.NoMDEntries; i++) {
        LOG_DEBUG("Record[{}]:",i);
        LOG_DEBUG("MDEntryType:{:.{}}",md.ExtendFields.MDEntryEntity[i].MDEntryType,sizeof(md.ExtendFields.MDEntryEntity[i].MDEntryType));
        LOG_DEBUG("MDEntryPx:{}",md.ExtendFields.MDEntryEntity[i].MDEntryPx);
        LOG_DEBUG("MDEntrySize:{}",md.ExtendFields.MDEntryEntity[i].MDEntrySize);
        LOG_DEBUG("MDPriceLevel:{}",md.ExtendFields.MDEntryEntity[i].MDPriceLevel);
        LOG_DEBUG("NumberOfOrders:{}",md.ExtendFields.MDEntryEntity[i].NumberOfOrders);
        LOG_DEBUG("NoOrders:{}",md.ExtendFields.MDEntryEntity[i].NoOrders);
    }
}
