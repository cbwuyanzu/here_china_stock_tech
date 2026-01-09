// Demo to consume SZSE MarketData
//
// Created by dzg on 2025/12/30.
//

//sockaddr_in AF_INET connect sockaddr inet_addr
#include <arpa/inet.h>

#include "types.h"
#include "configuration.h"
#include "loggerSingleton.h"
#include "session.h"

#define BUFFER_SIZE 1024
#define TOTAL_STEP  5

Configuration cfg = {};
LoggerSingleton LoggerSingleton::instance("log/SzMd.log","trace");

int main() {
  // 不知道为啥 一注释掉就core
  auto loggerConsole = spdlog::stdout_color_mt("console");
  loggerConsole->flush_on(spdlog::level::info);
  loggerConsole->info("start reading config.ini");
  INIReader reader("config.ini");
  if (reader.ParseError() < 0) {
    loggerConsole->critical("Can't load 'test.ini'");
    return 1;
  }
  loggerConsole->info("finish reading config.ini");


  strcpy(cfg.szServerIP, reader.GetString("COMMON", "SERVER_IP", "127.0.0.1").c_str());
  cfg.iPort = reader.GetInteger("COMMON", "SRRVER_MD_PORT", 8888);
  strcpy(cfg.szLocalName, reader.GetString("LOGON", "SENDER_NAME", "DEFAULT_SENDER").c_str());
  strcpy(cfg.szTargetName, reader.GetString("LOGON", "RECEIVER_NAME", "DEFAULT_RECEIVER").c_str());
  cfg.iHeartBeat = reader.GetInteger("LOGON", "HEARBEATINT", 30);
  strcpy(cfg.szPassword, reader.GetString("LOGON", "PASSWORD", "DEFAULT_PASSWORD").c_str());
  strcpy(cfg.szVersion, reader.GetString("LOGON", "VERSION", "1.0.0").c_str());
  strcpy(cfg.logLevel, reader.GetString("LOG", "LOGLEVEL", "info").c_str());
  strcpy(cfg.logFile, reader.GetString("LOG", "LOGFILE", "log/test.log").c_str());
  LOG_INFO("cfg.szServerIP:{}", cfg.szServerIP);
  LOG_INFO("cfg.iPort:{}", cfg.iPort);
  LOG_INFO("cfg.szLocalName:{}", cfg.szLocalName);
  LOG_INFO("cfg.szTargetName:{}", cfg.szTargetName);
  LOG_INFO("cfg.iHeartBeat:{}", cfg.iHeartBeat);
  LOG_INFO("cfg.szPassword:{}", cfg.szPassword);
  LOG_INFO("cfg.szVersion:{}", cfg.szVersion);
  LOG_INFO("cfg.logLevel:{}", cfg.logLevel);
  LOG_INFO("cfg.logFile:{}", cfg.logFile);

  // 创建socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    LOG_CRITICAL("Socket creation failed");
    return 1;
  }
  // 设置服务器地址
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(cfg.iPort);
  serverAddr.sin_addr.s_addr = inet_addr(cfg.szServerIP);
  if (connect(clientSocket, (sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
    LOG_CRITICAL("Connection failed");
    close(clientSocket);
    return 1;
  }
  LOG_INFO("step:1/{}\tConnect to server", TOTAL_STEP);

  if (SendLogon(clientSocket, cfg) != 0) {
    LOG_CRITICAL("Logon failed");
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
  close(clientSocket);
  return 0;

}
