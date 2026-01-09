//
// Created by chend on 2026/1/6.
//

#include "loggerSingleton.h"


std::shared_ptr<spdlog::logger> LoggerSingleton::getLogger() {
    return logger_;
}

LoggerSingleton &LoggerSingleton::getInstance() {
    return instance;
}

LoggerSingleton::~LoggerSingleton() {
    logger_->flush();
}

LoggerSingleton::LoggerSingleton(const char* fileName, const char* logLevel) {
    // logger_ = spdlog::stdout_color_mt("console");
    // 1t Average Speed 285.53 lines/ms
    // 2t Average Speed 111.80 lines/ms
    // logger_ = spdlog::basic_logger_mt("basic_logger", fileName);

    // 1t Average Speed 228.59 lines/ms  slower than basic_logger why?
    // 2t Average Speed 110.42 lines/ms
    spdlog::init_thread_pool(32768, 1);
    logger_ = spdlog::basic_logger_mt<spdlog::async_factory>(
             "async_logger", fileName);

    // basic_logger
    // no set_pattern-> Average Speed 233.64 lines/ms
    // same pattern->   Average Speed 170.65 lines/ms  why??
    // logger_->set_pattern("%Y-%m-%d %H:%M:%S.%e| [%n] [%l] %v");
    // most expected pattern-> Average Speed 129.37 lines/ms
    logger_->set_pattern("%Y-%m-%d %H:%M:%S.%f|%n|%l|%P|%t|%v");

    auto logLevel_ = spdlog::level::from_str(logLevel);
    if (logLevel_ == spdlog::level::off) {
        logger_->warn("invalid log level: {}, using info", logLevel);
        logger_->flush();
        logLevel_ = spdlog::level::info;
    } else {
        logger_->info("logLevel: {}",spdlog::level::to_string_view(logLevel_).data());
        logger_->flush();
    }
    logger_->set_level(logLevel_);
    //spdlog::level::info-> almost flush every line->   Average Speed 7 lines/ms
    //spdlog::level::err->  almost not flush->          Average Speed 235.29 lines/ms
    logger_->flush_on(spdlog::level::err);
}