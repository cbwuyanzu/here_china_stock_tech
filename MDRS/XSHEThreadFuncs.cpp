//
// Created by chend on 2026/1/15.
//
#include "XSHEThreadFuncs.h"
#include "XSHEBusiness.h"


void szIoThreadFunc(const Configuration &cfg) {
    constexpr int TOTAL_STEP  = 5;
    int needReconnect = 1;
    int clientSocket = 0;
    while (!APPINSTANCE.getExitFlag()) {
        while (needReconnect && !APPINSTANCE.getExitFlag()) {
            if (myConnect(cfg.szServerIP,cfg.iPort, clientSocket)) {
                LOG_CRITICAL("step:1 myConnect failed");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                continue;
            }
            LOG_INFO("step:1/{}\tConnect to server", TOTAL_STEP);
            if (SendLogon(clientSocket, cfg.reqLogon) != 0) {
                LOG_CRITICAL("step:2 Logon failed");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                continue;
            }
            LOG_INFO("step:2/{}\tLogon Send", TOTAL_STEP);
            if (RecvLogon(clientSocket) != 0) {
                LOG_CRITICAL("Logon failed");
                std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                continue;
            }
            needReconnect = 0;
            LOG_INFO("step:3/{}\tLogon Done", TOTAL_STEP);
            LOG_INFO("step:4/{}\tStart Receive MarketData", TOTAL_STEP);
        }
        needReconnect = RecvMsg(clientSocket);
    }
    if (SendLogout(clientSocket) != 0) {
        LOG_CRITICAL("SendLogout failed");
    } else {
        LOG_INFO("step:-2/{}\tSendLogout", TOTAL_STEP);
    }
    LOG_INFO("step:-1/{}\tTo Exit", TOTAL_STEP);
    myClose(clientSocket);
}


void szBusinessThreadFunc(const Configuration &cfg) {
    LOG_INFO("szBusinessThreadFunc running");
    v5QueueData queueData;
    initializeMsgHandlers();
    while (!APPINSTANCE.getExitFlag()) {
        //不用sleep了, 因为pop中含了wait_for
        popAndParse(cfg.popTimeout,queueData);
    }
}