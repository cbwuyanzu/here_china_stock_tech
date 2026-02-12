//
// Created by chend on 2026/1/15.
//

#include <iostream>
#include "myFIFOQueue.h"

template<typename T>
bool MyFifoQueue<T>::push(const T &item){
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(item);
    }
    cond_not_empty_.notify_one();
    return true;
}

template<typename T>
bool MyFifoQueue<T>::pop(T &item){
    std::lock_guard<std::mutex> lock(mutex_);
    if (!queue_.empty()) {
        item = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }
    return false;
}

template<typename T>
bool MyFifoQueue<T>::try_pop(T &item, int timeout_ms) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);
    auto status = cond_not_empty_.wait_for(lock, timeout,[&](){return !queue_.empty();});
    //timeout
    if (status == false) {
        return false;
    }
    //这个判断可以删掉 因为上面return true了
    if (!queue_.empty()) {
        item = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        return true;
    }
    lock.unlock();
    std::cout << "waring: wake up but queue_ is empty!" << std::endl;
    return false;
}

template<typename T>
bool MyFifoQueue<T>::try_pop_p(T &item, int timeout_ms,int &timeout_cnt ,int &wakeup_cnt) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);
    auto status = cond_not_empty_.wait_for(lock, timeout,[&](){return !queue_.empty();});
    //timeout
    if (status == false) {
        ++timeout_cnt;
        return false;
    }
    ++wakeup_cnt;
    //这个判断可以删掉 因为上面return true了
    if (!queue_.empty()) {
        item = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        return true;
    }
    lock.unlock();
    std::cout << "waring: wake up but queue_ is empty!" << std::endl;
    return false;
}

template<typename T>
bool MyFifoQueue<T>::try_pop_np(T &item, int timeout_ms, int &timeout_cnt ,int &wakeup_cnt) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);
    auto status = cond_not_empty_.wait_for(lock, timeout);
    if (status == std::cv_status::timeout) {
        ++timeout_cnt;
        if (queue_.empty()) {
            lock.unlock();
            return false;
        }
        item = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        return true;
    }
    ++wakeup_cnt;
    if (!queue_.empty()) {
        item = std::move(queue_.front());
        queue_.pop_front();
        lock.unlock();
        return true;
    }
    lock.unlock();
    std::cout << "waring: wake up but queue_ is empty!" << std::endl;
    return false;
}

template<typename T>
bool MyFifoQueue<T>::try_pop_v_p(std::vector<T> &items, int timeout_ms,int &timeout_cnt, int &wakeup_cnt) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);
    //这样写简单多了
    auto status = cond_not_empty_.wait_for(lock, timeout,[&](){return !queue_.empty();});
    //timeout
    if (status == false) {
        ++timeout_cnt;
        return false;
    }
    ++wakeup_cnt;
    items.clear();
    while (!queue_.empty()) {
        auto item = std::move(queue_.front());
        items.emplace_back(std::move(item));
        queue_.pop_front();
    }
    lock.unlock();
    return !items.empty();
}

template<typename T>
bool MyFifoQueue<T>::try_pop_v_np(std::vector<T> &items, int timeout_ms,int &timeout_cnt, int &wakeup_cnt) {
    auto timeout = std::chrono::milliseconds(timeout_ms);
    std::unique_lock<std::mutex> lock(mutex_);
    //使用wait不判超时可能出现永远不会被唤醒
    //使用wait_for 不加prediction 若无生产生唤醒 等待时间较长
    auto status = cond_not_empty_.wait_for(lock, timeout);
    if (status == std::cv_status::timeout) {
        ++timeout_cnt;
        if (queue_.empty()) return false;
        items.clear();
        while (!queue_.empty()) {
            auto item = std::move(queue_.front());
            items.emplace_back(std::move(item));
            queue_.pop_front();
        }
        lock.unlock();
        return !items.empty();
    }
    ++wakeup_cnt;
    items.clear();
    while (!queue_.empty()) {
        auto item = std::move(queue_.front());
        items.emplace_back(std::move(item));
        queue_.pop_front();
    }
    lock.unlock();
    return !items.empty();
}

template class MyFifoQueue<RawSzMDData>;
template class MyFifoQueue<v5QueueData>;