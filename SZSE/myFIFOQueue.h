//
// Created by chend on 2026/1/15.
//

#ifndef SZSE_MYFIFOQUEUE_H
#define SZSE_MYFIFOQUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include "types.h"

template<typename T>
class MyFifoQueue {
    mutable std::mutex mutex_;
    std::condition_variable cond_not_empty_;
    std::deque<T> queue_;

public:
    MyFifoQueue() = default;

    ~MyFifoQueue() = default;

    MyFifoQueue(const MyFifoQueue &) = delete;

    MyFifoQueue &operator=(const MyFifoQueue &) = delete;

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    bool push(const T &item);

    bool pop(T &item);
    bool try_pop(T &item,int timeout_ms); //->try_pop_p

    //前者测试效果更好, 唤醒数量占比只1%左右
    bool try_pop_p(T &item, int timeout_ms,int &timeout_cnt ,int &wakeup_cnt) ;
    bool try_pop_np(T &item, int timeout_ms,int &timeout_cnt ,int &wakeup_cnt) ;

    //这两个差不多，测下来没什么区别
    bool try_pop_v_p(std::vector<T> &items, int timeout_ms, int &timeout_cnt ,int &wakeup_cnt) ;
    bool try_pop_v_np(std::vector<T> &items, int timeout_ms, int &timeout_cnt,int &wakeup_cnt) ;

};

#endif //SZSE_MYFIFOQUEUE_H
