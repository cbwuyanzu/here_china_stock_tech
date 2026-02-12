// Demo to consume MDRS MarketData
//
// Created by dzg on 2025/12/30.
//

// dependency comment
// XSHETypes.h           -> configuration.h
// loggerSingleton.h ->

// utility.h -> session.h

#include "XSHEBusiness.h"
#include "myPub/myInih.h"
#include "myPub/loggerSingleton.h"
#include "XSHEThreadFuncs.h"
#include "appManager.h"

#define BUFFER_SIZE 1024

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
  std::thread tXSHEIoThread(szIoThreadFunc,std::ref(cfg));
  std::thread tXSHEBusinessThread(szBusinessThreadFunc,std::ref(cfg));
  char cmd = 0;
  APPINSTANCE.printCmd();
  while (!APPINSTANCE.getExitFlag()) {
    scanf("%c",&cmd);
    if (cmd == '\n') {
      continue;
    }
    switch (cmd) {
      case 'q':
      case 'Q':
        APPINSTANCE.setExitFlag(1);
        break;
      case 's':
      case 'S':
        APPINSTANCE.show();
        break;
      case 'd':
      case 'D':
        APPINSTANCE.dump();
        break;
      case 'c':
      case 'C':
        APPINSTANCE.clear();
      default: ;
    }
    APPINSTANCE.printCmd();
  }
  tXSHEIoThread.join();
  tXSHEBusinessThread.join();
  return 0;
}