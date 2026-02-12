//
// Created by chend on 2026/1/23.
//

#ifndef SZSE_SHMSTORAGE_H
#define SZSE_SHMSTORAGE_H


// #include <cstdint>
// #include <cstdlib>

#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <mutex>

#include "../XSHETypes.h"

// 固定大小的简单哈希表（开放寻址，线性探测）
struct ShmRecord {
    MyMDItem myMDItem;
    bool valid;               // 槽位是否有效
};

struct ShmIndex {
    uint32_t recordId;
};

struct ShmCtrl {
    uint32_t size;
};

class ShmStorage {
private:
    constexpr static size_t TABLE_SIZE = 10*1000000;
    constexpr static size_t CTRL_SIZE = sizeof(ShmCtrl);
    constexpr static size_t INDEX_SIZE = sizeof(ShmIndex) * TABLE_SIZE;
    constexpr static size_t RECORD_SIZE = sizeof(ShmRecord) * TABLE_SIZE;
    constexpr static size_t TOTAL_SIZE = CTRL_SIZE + INDEX_SIZE + RECORD_SIZE;

    ShmRecord* table_;
    ShmIndex* index_;
    ShmCtrl* ctrl_;

    int fd_;

    std::mutex mutex_;

public:
    explicit ShmStorage(const char* shm_name) {
        // 创建共享内存
        fd_ = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
        ftruncate(fd_, TOTAL_SIZE);

        auto mmapRet = mmap(nullptr, TOTAL_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (mmapRet == MAP_FAILED) {
            perror("mmap failed");
            exit(-1);
        }
        ctrl_ = static_cast<ShmCtrl *>(mmapRet);
        index_ = reinterpret_cast<ShmIndex *>(ctrl_+1);
        table_ = reinterpret_cast<ShmRecord *>(index_+TABLE_SIZE);

        // 初始化 看看ctrl_与index_是否符合 不符合的话直接memset 0
        std::lock_guard<std::mutex> lock(mutex_);
        uint32_t size = ctrl_->size;
        bool needReset = false;
        for (int i = 0; i < TABLE_SIZE; i++) {
            if (i < size ) {
                if (index_[i].recordId >= TABLE_SIZE || table_[index_[i].recordId].valid == false) {
                    printf("needReset i:%d index_[i].recordId:%d table_[index_[i].recordId].valid:%d LINE:%d\n",
                        i,index_[i].recordId,table_[index_[i].recordId].valid,__LINE__);
                    needReset = true;
                    break;
                }
            } else {
                if (index_[i].recordId != 0 || table_[index_[i].recordId].valid == true ) {
                    printf("needReset i:%d index_[i].recordId:%d table_[index_[i].recordId].valid:%d LINE:%d\n",
                        i,index_[i].recordId,table_[index_[i].recordId].valid,__LINE__);
                    needReset = true;
                    break;
                }
            }
        }
        if (needReset) {
            ctrl_->size = 0;
            memset(index_, 0, INDEX_SIZE+RECORD_SIZE);
        }
    }

    ~ShmStorage() {
        munmap(table_, TOTAL_SIZE);
        close(fd_);
    }

    // 插入或更新
    void set(int key, const MyMDItem& myMDItem) {
        uint32_t h = key;
        std::lock_guard<std::mutex> lock(mutex_);
        if (table_[h].valid == false) {
            index_[ctrl_->size].recordId = h;
            ++ctrl_->size;
        }
        table_[h].valid = true;
        table_[h].myMDItem = myMDItem;
    }

    // 查找
    bool get(int key, MyMDItem& myMDItem) {
        uint32_t h = key;
        std::lock_guard<std::mutex> lock(mutex_);
        if (table_[h].valid == true) {
            myMDItem = table_[h].myMDItem;
            return true;
        }
        return false;
    }

    MyMDItem* getPtr(int key) {
        uint32_t h = key;
        std::lock_guard<std::mutex> lock(mutex_);
        if (table_[h].valid == true) {
            MyMDItem* myMDItem;
            myMDItem = &(table_[h].myMDItem);
            return myMDItem;
        }
        return nullptr;
    }

    bool clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        ctrl_->size = 0;
        memset(index_, 0, INDEX_SIZE+RECORD_SIZE);
        return true;
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return ctrl_->size;
    }

    template<typename Func>
    void forEach(Func func) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (int i = 0; i < ctrl_->size; i++) {
            if (table_[index_[i].recordId].valid == true) {
                func(table_[index_[i].recordId].myMDItem);
            }
        }
    }
};


#endif //SZSE_SHMSTORAGE_H