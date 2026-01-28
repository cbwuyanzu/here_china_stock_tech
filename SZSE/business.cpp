//
// Created by chend on 2026/1/19.
//

#include "business.h"
#include "myFIFOQueue.h"
#include "myFIFOQueueNL.h"

#include "utility.h"
#include "szMdParser.h"

extern  MDParser gMDParser;
extern MyFifoQueueNL<v5QueueData> queueForSZMarketData;

FuncStatMap fsBusiness;
std::mutex mtxBusiness;

int globalHKTradingSessionSubId = -1;

void popAndParse(int timeoutMs, v5QueueData &queueData){
    if (!queueForSZMarketData.try_pop(queueData,timeoutMs)) {
        return;
    }
    auto start = std::chrono::system_clock::now();
#if 0
    switch (queueData.parsedHead.MsgType) {
        case 300111: {
            RawSzMDData md = {};
            if(deserializeBody(md,queueData.notParsedBody.charArray,queueData.parsedHead.BodyLength)) {
                LOG_ERROR("deserialize body failed {}",__LINE__);
                break;
            }
            LogMdData(md);
            OnRealTimeMD(md);
            break;
        }
        case 390019: {
            RawSzHkMarketStatus ms = {};
            if (deserializeBody(ms,queueData.notParsedBody.charArray,queueData.parsedHead.BodyLength)) {
                LOG_ERROR("deserialize body failed {}",__LINE__);
                break;
            }
            LogMdData(ms);
            OnRealTimeHKMarketStatus(ms);
            break;
        }
        default: {
            ;
        }
    }
#else
    bool ret = MsgRouter::getInstance().route(queueData.parsedHead.MsgType,queueData);
#endif
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    {
        mtxBusiness.lock();
        auto it = fsBusiness.find(queueData.parsedHead.MsgType);
        if ( it == fsBusiness.end()) {
            fsBusiness.insert(std::make_pair(queueData.parsedHead.MsgType,funcStat{1,0,duration.count()}));
        } else {
            it->second.success++;
            it->second.successTimeCostUs += duration.count();
        }
        mtxBusiness.unlock();
    }
}

void OnRealTimeMD(const RawSzMDData &md) {
    gMDParser.parse(md);
}

void OnRealTimeMD(const RawSzHkMDData &md) {
    gMDParser.parse(md);
}

void OnRealTimeHKMarketStatus(const RawSzHkMarketStatus &ms) {
    int tmp = atoi(ms.TradingSessionSubID);
    if (globalHKTradingSessionSubId != tmp) {
        LOG_INFO("globalHKTradingSessionSubId changed [{}-{}] -> [{}-{}]",globalHKTradingSessionSubId,DictTradingSessionSubID.at(globalHKTradingSessionSubId),
                                                        tmp,DictTradingSessionSubID.at(tmp));
        globalHKTradingSessionSubId = tmp;
    }
}

void OnChannelStat(const RawSzChannelStat &cs) {

}


int deserializeBody(RawSzMDData &md,const void* buffer, uint32_t length) {
    auto *p = static_cast<const RawSzMDData*>(buffer);
    //这里先做一些包的检查, 以免引起coredump
    uint32_t uNoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    if ( uNoMDEntries > MAX_MD_ENTRY_NO) {
        return -1;
    }
    if (length != uNoMDEntries * sizeof(MDEntry) + reinterpret_cast<char *>(&md.ExtendFields.MDEntryEntity) -  reinterpret_cast<char *>(&md)) {
        LOG_ERROR("NoMDEntries:{}\tlength:{}\tExpect length:{}",
            md.ExtendFields.NoMDEntries, length,
            uNoMDEntries * sizeof(MDEntry) + reinterpret_cast<char *>(&md.ExtendFields.MDEntryEntity) -  reinterpret_cast<char *>(&md));
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


int deserializeBody(RawSzHkMDData &md,const void* buffer, uint32_t length) {
    auto *p = static_cast<const RawSzHkMDData*>(buffer);
    //这里先做一些包的检查, 以免引起coredump
    uint32_t uNoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    if ( uNoMDEntries > MAX_MD_ENTRY_NO) {
        return -1;
    }
    auto *pHkVcm = reinterpret_cast<const HkVcm*>(&p->ExtendFields.MDEntryEntity[uNoMDEntries]);
    uint32_t uNoComplexEventTimes = htnu32(pHkVcm->NoComplexEventTimes);
    if (length != uNoMDEntries * sizeof(HkMDEntry)
            + reinterpret_cast<char *>(&md.ExtendFields.MDEntryEntity) -  reinterpret_cast<char *>(&md)
            + sizeof(md.ExtendFields.hkVcm.NoComplexEventTimes)
            + uNoComplexEventTimes * (sizeof(md.ExtendFields.hkVcm.ComplexEventStartTime) + sizeof(md.ExtendFields.hkVcm.ComplexEventEndTime)))
    {
        LOG_ERROR("NoMDEntries:{}\tNoComplexEventTimes:{}\tlength:{}\tExpect length:{}",
            md.ExtendFields.NoMDEntries,uNoComplexEventTimes, length,
            uNoMDEntries * sizeof(HkMDEntry)
            + reinterpret_cast<char *>(&md.ExtendFields.MDEntryEntity) -  reinterpret_cast<char *>(&md)
            + sizeof(md.ExtendFields.hkVcm.NoComplexEventTimes)
            + uNoComplexEventTimes * (sizeof(md.ExtendFields.hkVcm.ComplexEventStartTime) + sizeof(md.ExtendFields.hkVcm.ComplexEventEndTime)));
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
    }
    md.ExtendFields.hkVcm.NoComplexEventTimes = htnu32(pHkVcm->NoComplexEventTimes);
    for (int i = 0; i < md.ExtendFields.hkVcm.NoComplexEventTimes; i++) {
        md.ExtendFields.hkVcm.ComplexEventStartTime = htn64(pHkVcm->ComplexEventStartTime);
        md.ExtendFields.hkVcm.ComplexEventEndTime = htn64(pHkVcm->ComplexEventEndTime);
    }
    return 0;
}

int deserializeBody(RawSzHkMarketStatus &ms,const void* buffer, uint32_t length) {
    auto *p = static_cast<const RawSzHkMarketStatus*>(buffer);
    if (length != sizeof(RawSzHkMarketStatus)) {
        LOG_ERROR("bodylength:{}, Expected sizeof(RawSzHkMarketStatus):{}",
            length, sizeof(RawSzHkMarketStatus));
        return -1;
    }
    memset(&ms, 0, sizeof(ms));
    memcpy(&ms, buffer, length);
    ms.OrigTime = htn64(p->OrigTime);
    ms.ChannelNo = htnu16(p->ChannelNo);
    ms.TradSesStatus = htnu16(p->TradSesStatus);
    ms.TradSesStartTime = htn64(p->TradSesStartTime);
    ms.TradSesEndTime = htn64(p->TradSesEndTime);
    ms.ThresholdAmount = htn64(p->ThresholdAmount);
    ms.PosAmt = htn64(p->PosAmt);
    return 0;
}

int deserializeBody(RawSzChannelStat &md,const void* buffer, uint32_t length) {
    auto *p = static_cast<const RawSzChannelStat*>(buffer);
    //这里先做一些包的检查, 以免引起coredump
    uint32_t noMDSteamId = htnu32(p->NoMDSteamId);
    if ( noMDSteamId > MAX_MD_ENTRY_NO) {
        return -1;
    }
    if (length != noMDSteamId * sizeof(ChannelStatEntry) + reinterpret_cast<char *>(&md.ChannelStatEntity) -  reinterpret_cast<char *>(&md)) {
        LOG_ERROR("NoMDSteamId:{}\tlength:{}\tExpect length:{}",
            noMDSteamId, length,
            noMDSteamId * sizeof(ChannelStatEntry)+ sizeof(NumInGroup) + reinterpret_cast<char *>(&md.ChannelStatEntity) -  reinterpret_cast<char *>(&md));
        return -2;
    }
    memset(&md, 0, sizeof(md));
    memcpy(&md, buffer, length);
    md.OrigTime = htn64(p->OrigTime);
    md.ChannelNo = htnu16(p->ChannelNo);
    md.NoMDSteamId = htnu32(p->NoMDSteamId);
    for (int i = 0; i < noMDSteamId; i++) {
        md.ChannelStatEntity[i].StockNum = htnu32(p->ChannelStatEntity[i].StockNum);
    }
    return 0;
}

void LogMdData(const RawSzMDData & md) {
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

void LogMdData(const RawSzHkMDData & md) {
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
    }
    LOG_DEBUG("NoComplexEventTimes:{}",md.ExtendFields.hkVcm.NoComplexEventTimes)
    for (int i = 0; i < md.ExtendFields.hkVcm.NoComplexEventTimes; i++) {
        LOG_DEBUG("ComplexEventStartTime[{}]:",md.ExtendFields.hkVcm.ComplexEventStartTime);
        LOG_DEBUG("ComplexEventEndTime[{}]:",md.ExtendFields.hkVcm.ComplexEventEndTime);
    }
}

void LogMdData(const RawSzHkMarketStatus & ms) {
    LOG_DEBUG("OrigTime:{}",ms.OrigTime);
    LOG_DEBUG("ChannelNo:{}",ms.ChannelNo);
    LOG_DEBUG("MarketId:{:.{}}",ms.MarketId, sizeof(ms.MarketId));
    LOG_DEBUG("MarketSegmentID:{:.{}}",ms.MarketSegmentID, sizeof(ms.MarketSegmentID));
    LOG_DEBUG("TradingSessionID:{:.{}}",ms.TradingSessionID, sizeof(ms.TradingSessionID));
    LOG_DEBUG("TradingSessionSubID:{:.{}} {}",ms.TradingSessionSubID, sizeof(ms.TradingSessionSubID), DictTradingSessionSubID.at(atoi(ms.TradingSessionSubID)));
    LOG_DEBUG("TradSesStatus:{}",ms.TradSesStatus);
    LOG_DEBUG("TradSesStartTime:{}",ms.TradSesStartTime);
    LOG_DEBUG("TradSesEndTime:{}",ms.TradSesEndTime);
    LOG_DEBUG("ThresholdAmount:{}",ms.ThresholdAmount);
    LOG_DEBUG("PosAmt:{}",ms.PosAmt)
    LOG_DEBUG("AmountStatus:{}",ms.AmountStatus)
}

void LogMdData(const RawSzChannelStat & md) {
    LOG_DEBUG("OrigTime:{}",md.OrigTime);
    LOG_DEBUG("ChannelNo:{}",md.ChannelNo);
    LOG_DEBUG("NoMDSteamId:{}",md.NoMDSteamId);
    for (int i = 0; i < md.NoMDSteamId; i++) {
        LOG_DEBUG("Record[{}]:",i);
        LOG_DEBUG("MDStreamID:{:.{}}",md.ChannelStatEntity[i].MDStreamID,sizeof(md.ChannelStatEntity[i].MDStreamID));
        LOG_DEBUG("StockNum:{}",md.ChannelStatEntity[i].StockNum);
        LOG_DEBUG("TradingPhaseCode:{:.{}}",md.ChannelStatEntity[i].TradingPhaseCode,sizeof(md.ChannelStatEntity[i].TradingPhaseCode));
    }
}



