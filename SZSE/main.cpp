// Demo to consume SZSE MarketData
//
// Created by dzg on 2025/12/30.
//

// dependency comment
// types.h           -> configuration.h
// loggerSingleton.h ->

// utility.h -> session.h

#include "types.h"
#include "configuration.h"
#include "loggerSingleton.h"
#include "session.h"
#include "utility.h"

#define BUFFER_SIZE 1024
#define TOTAL_STEP  5

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

  int clientSocket = 0;
  if (myConnect(cfg.szServerIP,cfg.iPort, clientSocket)) {
    LOG_CRITICAL("step:1 myConnect failed");
  }
  LOG_INFO("step:1/{}\tConnect to server", TOTAL_STEP);

  if (SendLogon(clientSocket, cfg.reqLogon) != 0) {
    LOG_CRITICAL("step:2 Logon failed");
    return 2;
  }
  LOG_INFO("step:2/{}\tLogon Send", TOTAL_STEP);

  if (RecvLogon(clientSocket) != 0) {
    LOG_CRITICAL("Logon failed");
    return 3;
  }
  LOG_INFO("step:3/{}\tLogon Done", TOTAL_STEP);
  LOG_INFO("step:4/{}\tStart Receive MarketData", TOTAL_STEP);
  int bexit = 0;
  while (bexit == 0) {
    bexit = RecvMsg(clientSocket);
  }
  LOG_INFO("step:last/{}\tTo Exit bexit=", TOTAL_STEP);
  myClose(clientSocket);
  return 0;
}