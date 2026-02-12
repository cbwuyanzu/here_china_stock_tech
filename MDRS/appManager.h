//
// Created by chend on 2026/1/28.
//

#ifndef SZSE_APPXSHEMANAGER_H
#define SZSE_APPXSHEMANAGER_H

#include "myPub/loggerSingleton.h"
#include "myPub/myFIFOQueueNL.h"
#include "XSHEMdParser.h"


class appManager {
private:
    MDParser mdParser;
    std::mutex mtxIo;
    std::mutex mtxBusiness;
    FuncStatMap fsIo;
    FuncStatMap fsBusiness;
    MyFifoQueueNL<v5QueueData> queueForXSHEMarketData;
    volatile int hkTradingSessionSubId{-1};
    volatile int exitFlag{0};
    appManager() = default;
    ~appManager() = default;

    //for XSHG
    MyFifoQueueNL<v5QueueData> queueForXSHGMarketData;
public:
    appManager(const appManager &) = delete;
    appManager(const appManager &&) = delete;
    appManager& operator =(const appManager &) = delete;
    appManager&& operator =(const appManager &&) = delete;
    static appManager& getInstance() {
        static appManager instance;
        return instance;
    }

    MyFifoQueueNL<v5QueueData>& getQueue() {
        return queueForXSHEMarketData;
    }

    std::mutex& getIoMutex() {
        return mtxIo;
    }
    std::mutex& getBusinessMutex() {
        return mtxBusiness;
    }

    MDParser& getMDParser() {
        return mdParser;
    }

    FuncStatMap& getIoFs() {
        return fsIo;
    }
    FuncStatMap& getBusinessFs() {
        return fsBusiness;
    }

    int getHkTradingSessionSubId() const {
        return hkTradingSessionSubId;
    }

    int getExitFlag() const {
        return exitFlag;
    }

    void setHkTradingSessionSubId(int iHkTradingSessionSubId) {
        hkTradingSessionSubId = iHkTradingSessionSubId;
    }

    void setExitFlag(int iExitFlag) {
        exitFlag = iExitFlag;
    }

    void initialize() {

    }

    void dump();

    void printCmd();

    void show();

    void clear();
};

#define APPINSTANCE appManager::getInstance()

#endif //SZSE_APPXSHEMANAGER_H