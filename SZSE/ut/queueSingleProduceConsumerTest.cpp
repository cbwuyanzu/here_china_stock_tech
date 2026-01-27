//
// Created by chend on 2026/1/15.
//

#include <iostream>
#include <thread>
#include <vector>
#include "spdlog/spdlog.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "myFIFOQueue.h"

constexpr int NumLoops = 50* 1000;
constexpr int PreSleepMs = 2000;
constexpr int LoopSleepMs = 1;

auto console_logger = spdlog::stdout_color_mt("console");

void handleConsume(RawSzMDData &data) {
    //Do nothing
}

using ConsumerFunc = std::function<void (MyFifoQueue<RawSzMDData>& , int , int ,volatile int &,int &, int &, int &)>;

void consumerThreadFuncPop(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag,int &cnt, int &timeoutCnt, int &wakeupCnt) {
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

void consumerThreadFuncTryPopVP(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag, int &tryCnt, int &timeoutCnt, int &wakeupCnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    std::vector<RawSzMDData> values;
    while (doneFlag == 0 || !queue.empty()) {
        tryCnt++;
        if (queue.try_pop_v_p(values,loopSleepMs,timeoutCnt,wakeupCnt)){
            for (auto &value:values) {
                handleConsume(value);
            }
        } else {
        }
    }
}

void consumerThreadFuncTryPopVNP(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag, int &tryCnt, int &timeoutCnt, int &wakeupCnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    std::vector<RawSzMDData> values;
    while (doneFlag == 0 || !queue.empty()) {
        tryCnt++;
        if (queue.try_pop_v_np(values,loopSleepMs,timeoutCnt,wakeupCnt)){
            for (auto &value:values) {
                handleConsume(value);
            }
        } else {
        }
    }
}

void consumerThreadFuncTryPopP(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag, int &tryCnt, int &timeoutCnt, int &wakeupCnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        tryCnt++;
        if (queue.try_pop_p(value,loopSleepMs,timeoutCnt,wakeupCnt)){
            handleConsume(value);
        } else {
        }
    }
}

void consumerThreadFuncTryPopNP(MyFifoQueue<RawSzMDData>& queue, int preSleepMs, int loopSleepMs,volatile int &doneFlag, int &tryCnt, int &timeoutCnt, int &wakeupCnt) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    RawSzMDData value;
    while (doneFlag == 0 || !queue.empty()) {
        tryCnt++;
        if (queue.try_pop_np(value,loopSleepMs, timeoutCnt,wakeupCnt)){
            handleConsume(value);
        } else {
        }
    }
}

void producerThreadFunc(MyFifoQueue<RawSzMDData> &queue,int preSleepMs, int loopSleepMs, volatile int &doneFlag) {
    if (preSleepMs > 0)
        std::this_thread::sleep_for(std::chrono::milliseconds(preSleepMs));
    for (int i = 1; i <= NumLoops; i++) {
        queue.push(RawSzMDData{i});
        if (loopSleepMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(loopSleepMs));
    }
    doneFlag = 1;
    console_logger->debug("produce done!");
}

struct TestDefine {
    std::string name;
    ConsumerFunc consumerFunc;
    int consumerDelayMs;
    int consumerLoopWaitMs;
    int producerDelayMs;
    int producerLoopWaitMs;
};

void runGenericTest(TestDefine testDefine, int &timeoutCnt, int &wakeupCnt, int &tryCnt) {
    MyFifoQueue<RawSzMDData> q;
    volatile int doneFlag = 0;

    std::thread consumerThread([&]() {testDefine.consumerFunc(std::ref(q),
            testDefine.consumerDelayMs, testDefine.consumerLoopWaitMs,std::ref(doneFlag),
            std::ref(tryCnt), std::ref(timeoutCnt), std::ref(wakeupCnt));
    });

    std::thread producerThread(producerThreadFunc,std::ref(q),
            testDefine.producerDelayMs, testDefine.producerLoopWaitMs, std::ref(doneFlag));

    consumerThread.join();
    producerThread.join();
    assert(q.empty());
}


const TestDefine gTestDefine[] = {
    {"testConsumePopBeforeProduceFuc", consumerThreadFuncPop, 0, LoopSleepMs ,PreSleepMs, 0},
    {"testConsumeTryPopVPBeforeProduceFuc", consumerThreadFuncTryPopVP, 0, LoopSleepMs ,PreSleepMs, 0},
    {"testConsumeTryPopVNPBeforeProduceFuc", consumerThreadFuncTryPopVNP, 0, LoopSleepMs ,PreSleepMs, 0},
    {"testConsumeTryPopPBeforeProduceFuc", consumerThreadFuncTryPopP, 0, LoopSleepMs ,PreSleepMs, 0},
    {"testConsumeTryPopNPBeforeProduceFuc", consumerThreadFuncTryPopNP, 0, LoopSleepMs ,PreSleepMs, 0},

    {"testConsumePopWhileProduceFuc", consumerThreadFuncPop, 0, LoopSleepMs ,0, 0},
    {"testConsumeTryPopVPWhileProduceFuc", consumerThreadFuncTryPopVP, 0, LoopSleepMs ,0, 0},
    {"testConsumeTryPopVNPWhileProduceFuc", consumerThreadFuncTryPopVNP, 0, LoopSleepMs ,0, 0},
    {"testConsumeTryPopPWhileProduceFuc", consumerThreadFuncTryPopP, 0, LoopSleepMs ,0, 0},
    {"testConsumeTryPopNPWhileProduceFuc", consumerThreadFuncTryPopNP, 0, LoopSleepMs ,0, 0},

    {"testConsumePopAfterProduceFuc", consumerThreadFuncPop, PreSleepMs, LoopSleepMs ,0, 0},
    {"testConsumeTryPopVPAfterProduceFuc", consumerThreadFuncTryPopVP, PreSleepMs, LoopSleepMs ,0, 0},
    {"testConsumeTryPopVNPAfterProduceFuc", consumerThreadFuncTryPopVNP, PreSleepMs, LoopSleepMs ,0, 0},
    {"testConsumeTryPopPAfterProduceFuc", consumerThreadFuncTryPopP, PreSleepMs, LoopSleepMs ,0, 0},
    {"testConsumeTryPopNPAfterProduceFuc", consumerThreadFuncTryPopNP, PreSleepMs, LoopSleepMs ,0, 0}
};

template<typename  TestFunc>
void runTest(TestFunc testFunc, const std::string& testName) {
    console_logger->debug("{} start",testName);
    int try_cnt = 0;
    int timeout_cnt = 0;
    int wakeup_cnt = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    testFunc(timeout_cnt,wakeup_cnt,try_cnt);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    console_logger->info("{:40} completed! Produced {}, Try {:10}, Timeout {:8}, Wakeup {:8}, cost {:6} ms,  sleep(no while) {} ms",
                            testName,NumLoops, try_cnt,timeout_cnt,wakeup_cnt, duration.count(), PreSleepMs);
    console_logger->flush();
}

std::string stage[] = {
    "BEFORE",
    "WHILE",
    "AFTER"
};

int main() {
    console_logger->set_level(spdlog::level::info);
    console_logger->flush_on(spdlog::level::info);
    int i = 0;
    constexpr int groupSize = sizeof(gTestDefine)/sizeof(gTestDefine[0])/(sizeof(stage)/sizeof(stage[0]));
    for (auto testDefine:gTestDefine) {
        if (i++ % groupSize == 0) {
            console_logger->info("QueueTest Consume {} Produce start! numLoops {}",stage[i/groupSize], NumLoops);
        }
        runTest([&](int &timeoutCnt, int &wakeupCnt,int&tryCnt) {
                runGenericTest(testDefine,timeoutCnt,wakeupCnt,tryCnt);
            },
            testDefine.name);
    }
    return 0;
}

#if 0
numLoops 10000000
******
    BETTER
    queue
******
Total time: 7018 ms
******
    BETTER
    dequeue
******
Total time: 6474 ms

******
    SIMILAR
******
vector outside  MyFifoQueue
[2026-01-16 17:15:25.406] [console] [info] Produced 10000000 loops 143570, cost: 9991 ms including sleeping 2000 ms, testConsumeTryPopVBeforeProduceFuc completed!
[2026-01-16 17:15:33.521] [console] [info] Produced 10000000 loops 144388, cost: 8114 ms including sleeping 2000 ms, testConsumeTryPopVWhileProduceFuc completed!
[2026-01-16 17:15:46.125] [console] [info] Produced 10000000 loops 122487, cost: 12604 ms including sleeping 2000 ms, testConsumeTryPopVAfterProduceFuc completed!
vector inside  MyFifoQueue
[2026-01-16 17:11:57.549] [console] [info] Produced 10000000 loops 140712, cost: 9865 ms including sleeping 2000 ms, testConsumeTryPopVBeforeProduceFuc completed!
[2026-01-16 17:12:05.234] [console] [info] Produced 10000000 loops 134719, cost: 7685 ms including sleeping 2000 ms, testConsumeTryPopVWhileProduceFuc completed!
[2026-01-16 17:12:18.283] [console] [info] Produced 10000000 loops 120521, cost: 13048 ms including sleeping 2000 ms, testConsumeTryPopVAfterProduceFuc completed!

******
    SIMILAR
******
vector wait_for(lock, timeout)
[2026-01-16 18:25:35.464] [console] [info] Produced 1000000, Consuming Loops 16389, cost 2846 ms,  before/after sleeping 2000 ms, testConsumeTryPopVBeforeProduceFuc completed!
[2026-01-16 18:25:36.240] [console] [info] Produced 1000000, Consuming Loops 13463, cost 776 ms,  before/after sleeping 2000 ms, testConsumeTryPopVWhileProduceFuc completed!
[2026-01-16 18:25:40.244] [console] [info] Produced 1000000, Consuming Loops 1, cost 4003 ms,  before/after sleeping 2000 ms, testConsumeTryPopVAfterProduceFuc completed!
vector wait_for(lock, timeout, predicate)
[2026-01-16 18:23:03.095] [console] [info] Produced 1000000, Consuming Loops 15019, cost 2793 ms,  before/after sleeping 2000 ms, testConsumeTryPopVBeforeProduceFuc completed!
[2026-01-16 18:23:03.872] [console] [info] Produced 1000000, Consuming Loops 13790, cost 776 ms,  before/after sleeping 2000 ms, testConsumeTryPopVWhileProduceFuc completed!
[2026-01-16 18:23:07.863] [console] [info] Produced 1000000, Consuming Loops 1, cost 3990 ms,  before/after sleeping 2000 ms, testConsumeTryPopVAfterProduceFuc completed!

******
    WORSE
    element wait_for(lock, timeout)
    BETTER
    element wait_for(lock, timeout, predicate)
******
[2026-01-17 12:45:06.409] [console] [info] testConsumeTryPopNPBeforeProduceFuc      completed! Produced 10000, Try    11836, Timeout    11694, Wakeup      142, cost  12749 ms,  sleep(no while) 2000 ms
[2026-01-17 12:45:08.423] [console] [info] testConsumeTryPopPBeforeProduceFuc       completed! Produced 10000, Try    11840, Timeout     1840, Wakeup    10000, cost   2013 ms,  sleep(no while) 2000 ms
[2026-01-17 12:45:19.141] [console] [info] testConsumeTryPopNPWhileProduceFuc       completed! Produced 10000, Try    10000, Timeout     9845, Wakeup      155, cost  10717 ms,  sleep(no while) 2000 ms
[2026-01-17 12:45:19.150] [console] [info] testConsumeTryPopPWhileProduceFuc        completed! Produced 10000, Try    10000, Timeout        0, Wakeup    10000, cost      8 ms,  sleep(no while) 2000 ms
[2026-01-17 12:45:32.025] [console] [info] testConsumeTryPopNPAfterProduceFuc       completed! Produced 10000, Try    10000, Timeout    10000, Wakeup        0, cost  12875 ms,  sleep(no while) 2000 ms
[2026-01-17 12:45:34.028] [console] [info] testConsumeTryPopPAfterProduceFuc        completed! Produced 10000, Try    10000, Timeout        0, Wakeup    10000, cost   2002 ms,  sleep(no while) 2000 ms

******
    ALL
******
[2026-01-17 15:36:40.569] [console] [info] QueueTest Consume BEFORE Produce start! numLoops 50000
[2026-01-17 15:36:42.612] [console] [info] testConsumePopBeforeProduceFuc           completed! Produced 50000, Try      51834, Timeout        0, Wakeup        0, cost   2042 ms,  sleep(no while) 2000 ms
[2026-01-17 15:36:44.656] [console] [info] testConsumeTryPopVPBeforeProduceFuc      completed! Produced 50000, Try       2595, Timeout     1832, Wakeup      763, cost   2044 ms,  sleep(no while) 2000 ms
[2026-01-17 15:36:46.699] [console] [info] testConsumeTryPopVNPBeforeProduceFuc     completed! Produced 50000, Try       2635, Timeout     1832, Wakeup      803, cost   2042 ms,  sleep(no while) 2000 ms
[2026-01-17 15:36:48.736] [console] [info] testConsumeTryPopPBeforeProduceFuc       completed! Produced 50000, Try      51832, Timeout     1832, Wakeup    50000, cost   2036 ms,  sleep(no while) 2000 ms
[2026-01-17 15:37:44.736] [console] [info] testConsumeTryPopNPBeforeProduceFuc      completed! Produced 50000, Try      51828, Timeout    51060, Wakeup      768, cost  56000 ms,  sleep(no while) 2000 ms
[2026-01-17 15:37:44.736] [console] [info] QueueTest Consume WHILE Produce start! numLoops 50000
[2026-01-17 15:37:44.783] [console] [info] testConsumePopWhileProduceFuc            completed! Produced 50000, Try      50002, Timeout        0, Wakeup        0, cost     46 ms,  sleep(no while) 2000 ms
[2026-01-17 15:37:44.828] [console] [info] testConsumeTryPopVPWhileProduceFuc       completed! Produced 50000, Try        837, Timeout        0, Wakeup      837, cost     45 ms,  sleep(no while) 2000 ms
[2026-01-17 15:37:44.873] [console] [info] testConsumeTryPopVNPWhileProduceFuc      completed! Produced 50000, Try        841, Timeout        0, Wakeup      841, cost     44 ms,  sleep(no while) 2000 ms
[2026-01-17 15:37:44.913] [console] [info] testConsumeTryPopPWhileProduceFuc        completed! Produced 50000, Try      50000, Timeout        0, Wakeup    50000, cost     39 ms,  sleep(no while) 2000 ms
[2026-01-17 15:38:38.945] [console] [info] testConsumeTryPopNPWhileProduceFuc       completed! Produced 50000, Try      50000, Timeout    49312, Wakeup      688, cost  54032 ms,  sleep(no while) 2000 ms
[2026-01-17 15:38:38.945] [console] [info] QueueTest Consume AFTER Produce start! numLoops 50000
[2026-01-17 15:38:40.961] [console] [info] testConsumePopAfterProduceFuc            completed! Produced 50000, Try      50000, Timeout        0, Wakeup        0, cost   2015 ms,  sleep(no while) 2000 ms
[2026-01-17 15:38:43.059] [console] [info] testConsumeTryPopVPAfterProduceFuc       completed! Produced 50000, Try          1, Timeout        0, Wakeup        1, cost   2097 ms,  sleep(no while) 2000 ms
[2026-01-17 15:38:45.160] [console] [info] testConsumeTryPopVNPAfterProduceFuc      completed! Produced 50000, Try          1, Timeout        1, Wakeup        0, cost   2101 ms,  sleep(no while) 2000 ms
[2026-01-17 15:38:47.171] [console] [info] testConsumeTryPopPAfterProduceFuc        completed! Produced 50000, Try      50000, Timeout        0, Wakeup    50000, cost   2010 ms,  sleep(no while) 2000 ms
[2026-01-17 15:39:44.198] [console] [info] testConsumeTryPopNPAfterProduceFuc       completed! Produced 50000, Try      50000, Timeout    50000, Wakeup        0, cost  57027 ms,  sleep(no while) 2000 ms
#endif