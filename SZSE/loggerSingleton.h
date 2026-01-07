//
// Created by chend on 2026/1/6.
//

#ifndef SZSE_LOGGERSINGLETON_H
#define SZSE_LOGGERSINGLETON_H

#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"


class LoggerSingleton {
public:
    static LoggerSingleton& getInstance();

    std::shared_ptr<spdlog::logger> getLogger();

    LoggerSingleton(const LoggerSingleton &) = delete;
    LoggerSingleton& operator=(const LoggerSingleton &) = delete;

private:
    LoggerSingleton();
    ~LoggerSingleton();

    std::shared_ptr<spdlog::logger> logger_;

};

#define LOGGER LoggerSingleton::getInstance().getLogger()
#define LOG_INFO(...) {LOGGER->info(__VA_ARGS__);}
#define LOG_WARNING(...) {LOGGER->warning(__VA_ARGS__);}
#define LOG_ERROR(...) {LOGGER->error(__VA_ARGS__);}
#define LOG_CRITICAL(...) {LOGGER->critical(__VA_ARGS__);}


#endif
