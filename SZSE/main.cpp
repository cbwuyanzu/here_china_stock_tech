// Demo to consume SZSE MarketData
//
// Created by dzg on 2025/12/30.
//

// dependency comment
// types.h           -> configuration.h
// loggerSingleton.h ->

// utility.h -> session.h

#include "business.h"
#include "configuration.h"
#include "loggerSingleton.h"
#include "threadFuncs.h"

#define BUFFER_SIZE 1024

int exitFlag = 0;
LoggerSingleton LoggerSingleton::instance;


int main() {
  Configuration cfg = {};
  {
    auto loggerConsole = spdlog::stdout_color_mt("console");
    loggerConsole->info("start reading config.ini");
    INIReader reader("config.ini");
    if (reader.ParseError() < 0) {
      loggerConsole->critical("Can't load 'test.ini'");
      return 1;
    }
    reader.parstToStruct(cfg);
    loggerConsole->info("finish reading config.ini");
    loggerConsole->info("cfg.logFilePath:{}, cfg.logLevel:{}", cfg.logFile, cfg.logLevel);
    loggerConsole->flush();
    LoggerSingleton::getInstance().logInit(cfg.logFile,cfg.logLevel);
    reader.showConfig(cfg);
  }
  std::thread szIoThread(szIoThreadFunc,cfg);
  std::thread szBusinessThread(szBusinessThreadFunc,cfg);
  char cmd = 0;
  printCmd();
  while (!exitFlag) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    scanf("%c",&cmd);
    switch (cmd) {
      case 'q':
      case 'Q':
        exitFlag = 1;
        break;
      case 's':
      case 'S':
        show();
        break;
      case 'd':
      case 'D':
        dump();
        break;
      default:
        ;
    }
  }
  szIoThread.join();
  szBusinessThread.join();
  return 0;
}