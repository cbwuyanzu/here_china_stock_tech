//
// Created by chend on 2026/1/27.
//
#include <iostream>
#include <thread>
#include <vector>
#include "spdlog/spdlog.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "../myPub/myFIFOQueue.h"
#include "../myPub/myFIFOQueueNL.h"

constexpr int NumLoops = 1000* 1000;
constexpr int PreSleepMs = 2000;
constexpr int LoopSleepMs = 1;
constexpr int LoopSleepUs = 10;

auto console_logger = spdlog::stdout_color_mt("console");

void handleConsume(RawSzMDData &data) {
    // if (data.OrigTime == NumLoops)
        // console_logger->info("handle Done");
    //Do nothing
}

void producerThreadFuncNL(MyFifoQueueNL<RawSzMDData> &queue,int preSleepMs, int LoopSleepUs, volatile int &doneFlag) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    for (int i = 1; i <= NumLoops; i++) {
        while (queue.push(RawSzMDData{i}) != true) {
            if (LoopSleepUs > 0)
                std::this_thread::sleep_for(std::chrono::microseconds(LoopSleepUs));
        };
    }
    doneFlag = 1;
    console_logger->debug("MyFifoQueueNL produce done!");
}

void producerThreadFunc(MyFifoQueue<RawSzMDData> &queue,int preSleepMs, int LoopSleepUs, volatile int &doneFlag) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    for (int i = 1; i <= NumLoops; i++) {
        queue.push(RawSzMDData{i});
        // if (LoopSleepUs > 0)
            // std::this_thread::sleep_for(std::chrono::microseconds(LoopSleepUs));
    }
    doneFlag = 1;
    console_logger->debug("MyFifoQueue produce done!");
}

void consumerThreadFuncPop(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag,int &cnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        cnt++;
        if (queue.pop(value)){
            handleConsume(value);
        } else {
            if (loopSleepMs > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(loopSleepMs));
        }
    }
}

void consumerThreadFuncPopNL(MyFifoQueueNL<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag,int &cnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        cnt++;
        if (queue.pop(value)){
            handleConsume(value);
        } else {
            if (loopSleepMs > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(loopSleepMs));
        }
    }
}

void consumerThreadFuncTryPop(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag,int &cnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        cnt++;
        if (queue.try_pop(value,loopSleepMs)){
            handleConsume(value);
        } else {
        }
    }
}

void consumerThreadFuncTryPopNL(MyFifoQueueNL<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag,int &cnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        cnt++;
        if (queue.try_pop(value,loopSleepMs)){
            handleConsume(value);
        } else {

        }
    }
}



template<typename QueueType>
using ConsumerFunc = std::function<void (QueueType&, int, int, volatile int&, int&)>;

template<typename QueueType>
using ProducerFunc = std::function<void (QueueType&, int, int, volatile int&)>;

struct TEST_DEFINE{
    std::string name;
    ConsumerFunc<MyFifoQueue<RawSzMDData>> consumerFunc;
    ConsumerFunc<MyFifoQueueNL<RawSzMDData>> consumerNLFunc;
    ProducerFunc<MyFifoQueue<RawSzMDData>> producerFunc;
    ProducerFunc<MyFifoQueueNL<RawSzMDData>> producerNLFunc;
    int consumerDelayMs;
    int consumerLoopWaitMs;
    int producerDelayMs;
    int producerLoopWaitMs;
};

const TEST_DEFINE gTestDefine[] = {
    {"pop before push",consumerThreadFuncPop,consumerThreadFuncPopNL,producerThreadFunc,producerThreadFuncNL,0, LoopSleepMs ,PreSleepMs, LoopSleepUs},
    {"try pop before push",consumerThreadFuncTryPop,consumerThreadFuncTryPopNL,producerThreadFunc,producerThreadFuncNL,0, LoopSleepMs ,PreSleepMs, LoopSleepUs},
    {"pop while push",consumerThreadFuncPop,consumerThreadFuncPopNL,producerThreadFunc,producerThreadFuncNL,0, LoopSleepMs ,0, LoopSleepUs},
    {"try pop while push",consumerThreadFuncTryPop,consumerThreadFuncTryPopNL,producerThreadFunc,producerThreadFuncNL,0, LoopSleepMs ,0, LoopSleepUs},
    {"pop after push",consumerThreadFuncPop,consumerThreadFuncPopNL,producerThreadFunc,producerThreadFuncNL,PreSleepMs, LoopSleepMs ,0, LoopSleepUs},
    {"try pop after push",consumerThreadFuncTryPop,consumerThreadFuncTryPopNL,producerThreadFunc,producerThreadFuncNL,PreSleepMs, LoopSleepMs ,0, LoopSleepUs},
};


void runTest(TEST_DEFINE td) {
    {
        int tryCnt = 0;
        MyFifoQueue<RawSzMDData> q;
        volatile int doneFlag = 0;
        console_logger->debug("{} start",td.name);
        auto startTime = std::chrono::high_resolution_clock::now();
        std::thread consumerThread(td.consumerFunc,std::ref(q),td.consumerDelayMs,td.consumerLoopWaitMs,std::ref(doneFlag),
                std::ref(tryCnt));
        std::thread producerThread(td.producerFunc,std::ref(q), td.producerDelayMs, td.producerLoopWaitMs, std::ref(doneFlag));
        consumerThread.join();
        producerThread.join();
        assert(q.empty());
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        console_logger->info("{:20} completed! Produced {}, Try {:10}, cost {:6} ms,  sleep(no while) {} ms",
                                td.name,NumLoops, tryCnt, duration.count(), PreSleepMs);
        console_logger->flush();
    }
    {
        int tryCnt = 0;
        MyFifoQueueNL<RawSzMDData> q(4095);
        volatile int doneFlag = 0;
        console_logger->debug("{} NL start",td.name);
        auto startTime = std::chrono::high_resolution_clock::now();
        std::thread consumerThread(td.consumerNLFunc,std::ref(q),td.consumerDelayMs,td.consumerLoopWaitMs,std::ref(doneFlag),
                std::ref(tryCnt));
        std::thread producerThread(td.producerNLFunc,std::ref(q), td.producerDelayMs, td.producerLoopWaitMs, std::ref(doneFlag));
        consumerThread.join();
        producerThread.join();
        assert(q.empty());
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime);
        console_logger->info("{:17} NL completed! Produced {}, Try {:10}, cost {:6} ms,  sleep(no while) {} ms",
                                td.name,NumLoops, tryCnt, duration.count(), PreSleepMs);
        console_logger->flush();
    }
}

int main() {
    console_logger->set_level(spdlog::level::info);
    console_logger->flush_on(spdlog::level::info);
    for (const auto& t:gTestDefine) {
        runTest(t);
    }
    return 0;
}

#if 0
pop
[2026-01-27 14:33:25.945] [console] [info] runTestQueue                             completed! Produced 1000000, Try    1020060, Timeout        0, Wakeup        0, cost    710 ms,  sleep(no while) 2000 ms
[2026-01-27 14:33:26.093] [console] [info] runTestQueueNL                           completed! Produced 1000000, Try    8860522, Timeout        0, Wakeup        0, cost    147 ms,  sleep(no while) 2000 ms
try_pop
[2026-01-27 14:41:19.115] [console] [info] runTestQueue                             completed! Produced 1000000, Try    1000000, Timeout        0, Wakeup        0, cost    818 ms,  sleep(no while) 2000 ms
[2026-01-27 14:41:19.280] [console] [info] runTestQueueNL                           completed! Produced 1000000, Try    1965160, Timeout        0, Wakeup        0, cost    164 ms,  sleep(no while) 2000 ms

#endif
