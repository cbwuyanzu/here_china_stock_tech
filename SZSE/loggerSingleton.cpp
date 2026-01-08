//
// Created by chend on 2026/1/6.
//

#include "loggerSingleton.h"


std::shared_ptr<spdlog::logger> LoggerSingleton::getLogger() {
    return logger_;
}

LoggerSingleton &LoggerSingleton::getInstance() {
    static LoggerSingleton instance;
    return instance;
}

LoggerSingleton::~LoggerSingleton() {
    logger_->flush();
}

LoggerSingleton::LoggerSingleton() {
    logger_ = spdlog::basic_logger_mt("basic_logger", "log/SzMd1.log");
    logger_->flush_on(spdlog::level::info);
    logger_->set_level(spdlog::level::debug);
}
#if 0
LoggerSingleton::LoggerSingleton() {
    //TODO 最简单的console logger
    //auto logger = spdlog::stdout_color_mt("console");

    // logger_ = spdlog::basic_logger_mt("basic_logger", "log/SzMd.log");
    logger_->flush_on(spdlog::level::info);

    std::vector<spdlog::sink_ptr> sinks;
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "log/SzMd.log");
    file_sink->set_level(spdlog::level::debug);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
    sinks.push_back(file_sink);
    logger_ = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());

    //TODO 异步logger目前调试还有点问题
    //spdlog::init_thread_pool(8192, 1);
    //spdlog::flush_every(std::chrono::seconds(3));
    //auto logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_logger", "log/SzMd.log");
}
#endif