//
// Created by chend on 2026/1/19.
//

#ifndef SZSE_BUSINESS_H
#define SZSE_BUSINESS_H

#include <mutex>
#include "types.h"
#include "loggerSingleton.h"

void popAndParse(int timeoutMs, v5QueueData &queueData);

int deserializeBody(RawSzMDData &md,const void* buffer, uint32_t length);
int deserializeBody(RawSzHkMarketStatus &ms,const void* buffer, uint32_t length);
int deserializeBody(RawSzChannelStat &cs,const void* buffer, uint32_t length);

void LogMdData(const RawSzMDData & md);
void LogMdData(const RawSzHkMarketStatus & ms);
void LogMdData(const RawSzChannelStat & cs);


void OnRealTimeMD(const RawSzMDData &md);
void OnRealTimeHKMarketStatus(const RawSzHkMarketStatus &ms);
void OnChannelStat(const RawSzChannelStat &cs);

template<typename DataType>
struct MsgProcessor {
    using DeserializeFunc = int(*)(DataType&, const void*, uint32_t);
    using LogFunc = void(*)(const DataType&);
    using ProcessFunc = void(*)(const DataType&);

    DeserializeFunc deserialize;
    LogFunc logFunc;
    ProcessFunc process;

    bool operator()(v5QueueData& queueData) const {
        DataType data = {};
        if (deserialize(data, queueData.notParsedBody.charArray, queueData.parsedHead.BodyLength) != 0) {
            LOG_ERROR("deserialize body failed for msg type: {}", queueData.parsedHead.MsgType);
            return false;
        }
        if (logFunc != nullptr)
            logFunc(data);
        process(data);
        return true;
    }
};

class MsgRouter {
private:
    using HandlerFunc = std::function<bool(v5QueueData&)>;
    std::unordered_map<int, HandlerFunc> handlers;
    MsgRouter() = default;
    ~MsgRouter() = default;

public:
    template<typename DataType>
    void registerHandler(int msgType,
                        typename MsgProcessor<DataType>::DeserializeFunc deserialize,
                        typename MsgProcessor<DataType>::LogFunc logFunc,
                        typename MsgProcessor<DataType>::ProcessFunc process) {
        MsgProcessor<DataType> msgProcessFuncs{deserialize, logFunc,process};
        handlers[msgType] = [msgProcessFuncs](v5QueueData queueData) {
            return msgProcessFuncs(queueData);
        };
    }

    bool route(int msgType, v5QueueData& queueData) const {
        auto it = handlers.find(msgType);
        return it != handlers.end() && it->second(queueData);
    }

    bool contains(int msgType) const {
        return handlers.find(msgType) != handlers.end();
    }

    MsgRouter(const MsgRouter&) = delete;
    MsgRouter& operator=(const MsgRouter&) = delete;

    static MsgRouter instance;

    static MsgRouter& getInstance() {
        return instance;
    }
};

inline void initializeMsgHandlers() {
    MsgRouter::getInstance().registerHandler<RawSzMDData>(300111, deserializeBody,nullptr,OnRealTimeMD );
    MsgRouter::getInstance().registerHandler<RawSzHkMarketStatus>(390019,deserializeBody,nullptr,OnRealTimeHKMarketStatus);
    MsgRouter::getInstance().registerHandler<RawSzChannelStat>(390090,deserializeBody,LogMdData,OnChannelStat);
}

#endif //SZSE_BUSINESS_H