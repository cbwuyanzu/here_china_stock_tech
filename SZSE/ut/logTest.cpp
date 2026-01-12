//
// Created by chend on 2026/1/9.
//

#include <thread>
#include <vector>
#include "loggerSingleton.h"
#include "../loggerSingleton.h"

constexpr int numLoops = 1000000;

void testFuc(int thrdId) {
    for (int i = 1; i <= numLoops; i++) {
        LOG_INFO("thrdId:{}",thrdId);
    }
}

LoggerSingleton LoggerSingleton::instance;

int main() {
    auto console_logger = spdlog::stdout_color_mt("console");
    console_logger->set_level(spdlog::level::info);
    console_logger->flush_on(spdlog::level::info);
    LoggerSingleton::getInstance().logInit("logTest.log","info");
    constexpr int numThreads = 2;
    std::vector<std::thread> threadList;
    threadList.reserve(numThreads);
    console_logger->info("Test start!");
    console_logger->info("numThreads {}",numThreads);
    console_logger->info("numLoops {}",numLoops);
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; i++) {
        std::thread t1;
        threadList.emplace_back(testFuc, i);
    }
    for (auto& t:threadList) {
        t.join();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime);
    console_logger->info("Test completed!");
    console_logger->info("Total time: {} ms", duration.count());
    console_logger->info("Average Speed {:.2f} lines/ms", static_cast<double>(numLoops)*numThreads/duration.count());
    return 0;
}