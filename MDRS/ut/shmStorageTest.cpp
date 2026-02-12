//
// Created by chend on 2026/1/23.
//
#include "../myPub/shmStorage.h"
#include "../myPub/utility.h"

constexpr int LoopCnt = 9999999;

int main(){
    {
        ShmStorage ss("shmStorageTest.shm");
        auto start = std::chrono::high_resolution_clock::now();
        ss.clear();
        for (int i = 0; i <= LoopCnt; i++) {
            ss.set(i,MyMDItem{i/1000000,i%1000000});
        }
        MyMDItem item{};
        for (int i = 0; i <= LoopCnt; i++) {
            ss.get(i,item);
            item.updateTime++;
            ss.set(i,item);
        }
        ss.forEach([&](MyMDItem& record) {
            if (record.marketCode == 1 && record.stockCode == 600837) {
                printf("test 1 %d %d %lld\n",record.marketCode, record.stockCode, record.updateTime);
            }
        });
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("%20s loop %d cost %06ld ms\n","ShmStorage", LoopCnt,duration.count());
    }
    {
        ShmStorage ss("ShmStoragePtrTest.shm");
        auto start = std::chrono::high_resolution_clock::now();
        ss.clear();
        for (int i = 0; i <= LoopCnt; i++) {
            ss.set(i,MyMDItem{i/1000000,i%1000000});
        }
        MyMDItem* item;
        for (int i = 0; i <= LoopCnt; i++) {
            item = ss.getPtr(i);
            item->updateTime++;
        }
        ss.forEach([&](MyMDItem& record) {
            if (record.marketCode == 1 && record.stockCode == 600837) {
                printf("test 2 %d %d %lld\n",record.marketCode, record.stockCode, record.updateTime);
            }
        });
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("%20s loop %d cost %06ld ms\n","ShmStoragePtr", LoopCnt,duration.count());
    }
    {
        std::mutex mtx;
        std::unordered_map<int, MyMDItem> ss;
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i <= LoopCnt; i++) {
            std::lock_guard<std::mutex> lock(mtx);
            ss.insert({i,MyMDItem{i/1000000,i%1000000}});
        }
        MyMDItem item{};
        for (int i = 0; i <= LoopCnt; i++) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = ss.find(i);
            if (it != ss.end()) {
                it->second.updateTime++;
            }
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto it: ss) {
                if (it.first == 600837) {
                    printf("test 3 %d %d %lld\n",it.second.marketCode, it.second.stockCode, it.second.updateTime);
                }
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("%20s loop %d cost %06ld ms\n","std::unordered_map" ,LoopCnt , duration.count());
    }
    return 0;
}

