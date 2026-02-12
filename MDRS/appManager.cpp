//
// Created by chend on 2026/1/28.
//

#include "appManager.h"


void appManager::printCmd() {
    printf("press your cmd:\n");
    printf("  Q      Quit\n");
    printf("  S      Show\n");
    printf("  D      Dump\n");
    printf("  C      Clear\n");
    printf("\n");
}

void appManager::show() {
    mtxIo.lock();
    printf("IoThreadStat\nfuncNo|success|fail|successCostUs|failCostUs|aveSuccessCost|aveFailCost\n");
    for (auto p: fsIo) {
        printf("%d|%lld|%lld|%lld|%lld|"
               "%lld|%lld\n",
            p.first, p.second.success, p.second.fail, p.second.successTimeCostUs, p.second.failTimeCostUs,
            p.second.success!=0 ? p.second.successTimeCostUs/p.second.success:0, p.second.fail != 0 ? p.second.failTimeCostUs/p.second.fail:0);
    }
    mtxIo.unlock();
    mtxBusiness.lock();
    printf("BusinessThreadStat\nfuncNo|success|fail|successCostUs|failCostUs|aveSuccessCost|aveFailCost\n");
    for (auto p: fsBusiness) {
        printf("%d|%lld|%lld|%lld|%lld|"
               "%lld|%lld\n",
            p.first, p.second.success, p.second.fail, p.second.successTimeCostUs, p.second.failTimeCostUs,
            p.second.success!=0 ? p.second.successTimeCostUs/p.second.success:0, p.second.fail != 0 ? p.second.failTimeCostUs/p.second.fail:0);
    }
    mtxBusiness.unlock();
    printf("Connect Status: not finished yet\n");
    printf("\n");
}

void appManager::dump() {
    mdParser.dump("dump.txt");
}

void appManager::clear() {
    mdParser.clear();
}


