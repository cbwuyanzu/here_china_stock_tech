//
// Created by chend on 2026/1/28.
//

#ifndef SZSE_APPXSHEMANAGER_H
#define SZSE_APPXSHEMANAGER_H

#include "loggerSingleton.h"
#include "myFIFOQueueNL.h"
#include "XSHEMdParser.h"


class appXSHEManager {
private:
    MDParser mdParser;
    std::mutex mtxIo;
    std::mutex mtxBusiness;
    FuncStatMap fsIo;
    FuncStatMap fsBusiness;
    MyFifoQueueNL<v5QueueData> queueForXSHEMarketData;
    volatile int hkTradingSessionSubId{-1};
    volatile int exitFlag{0};
    appXSHEManager() = default;
    ~appXSHEManager() = default;
public:
    appXSHEManager(const appXSHEManager &) = delete;
    appXSHEManager(const appXSHEManager &&) = delete;
    appXSHEManager& operator =(const appXSHEManager &) = delete;
    appXSHEManager&& operator =(const appXSHEManager &&) = delete;
    static appXSHEManager& getInstance() {
        static appXSHEManager instance;
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

#define APPINSTANCE appXSHEManager::getInstance()

#endif //SZSE_APPXSHEMANAGER_H