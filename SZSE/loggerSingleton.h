//
// Created by chend on 2026/1/6.
//

#ifndef SZSE_LOGGERSINGLETON_H
#define SZSE_LOGGERSINGLETON_H

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"


class LoggerSingleton {
public:
    static LoggerSingleton& getInstance();

    std::shared_ptr<spdlog::logger> getLogger();
    LoggerSingleton(const LoggerSingleton &) = delete;
    LoggerSingleton& operator=(const LoggerSingleton &) = delete;

private:
    explicit LoggerSingleton(const char* fileName, const char* logLevel="info");
    ~LoggerSingleton();

    std::shared_ptr<spdlog::logger> logger_;
    static LoggerSingleton instance;
};

#define LOGGER LoggerSingleton::getInstance().getLogger()
#define LOG_DEBUG(...) {LOGGER->debug(__VA_ARGS__);}
#define LOG_INFO(...) {LOGGER->info(__VA_ARGS__);}
#define LOG_WARN(...) {LOGGER->warn(__VA_ARGS__);}
#define LOG_ERROR(...) {LOGGER->error(__VA_ARGS__);}
#define LOG_CRITICAL(...) {LOGGER->critical(__VA_ARGS__);}


#endif
