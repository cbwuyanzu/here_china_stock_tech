// Demo to consume SZSE MarketData
//
// Created by dzg on 2025/12/30.
//

#include <iostream>
#include <unistd.h>
#include "session.h"
#include "configuration.h"

#define BUFFER_SIZE 1024
#define TOTAL_STEP  10

Configuration cfg = {};

int main(){

  INIReader reader("config.ini");
  if (reader.ParseError() < 0) {
    std::cout << "Can't load 'test.ini'\n";
    return 1;
  }
  std::cout << "start reading config.ini\n";

  strcpy(cfg.szServerIP, reader.GetString("COMMON", "SERVER_IP", "127.0.0.1").c_str());
  cfg.iPort = reader.GetInteger("COMMON", "SRRVER_MD_PORT", 8888);
  strcpy(cfg.szLocalName, reader.GetString("LOGON", "SENDER_NAME", "DEFAULT_SENDER").c_str());
  strcpy(cfg.szTargetName, reader.GetString("LOGON", "RECEIVER_NAME", "DEFAULT_RECEIVER").c_str());
  cfg.iHeartBeat = reader.GetInteger("LOGON", "HEARBEATINT", 30);
  strcpy(cfg.szPassword, reader.GetString("LOGON", "PASSWORD", "DEFAULT_PASSWORD").c_str());
  strcpy(cfg.szVersion, reader.GetString("LOGON", "VERSION", "1.0.0").c_str());
  std::cout << cfg.szServerIP << "\n"
          << cfg.iPort << "\n"
          << cfg.szLocalName << "\n"
          << cfg.szTargetName << "\n"
          << cfg.iHeartBeat << "\n"
          << cfg.szPassword << "\n"
          << cfg.szVersion << "\n";

  // 创建socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    std::cerr << "Socket creation failed" << std::endl;
    return 1;
  }
   // 设置服务器地址
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(cfg.iPort);
  serverAddr.sin_addr.s_addr = inet_addr(cfg.szServerIP);
  
  if(connect(clientSocket,(sockaddr*)&serverAddr,sizeof(serverAddr)) == -1){
    std::cerr << "Connection failed" << std::endl;
    close(clientSocket);
    return 1;
  }
  std::cout << "step:1/"<< TOTAL_STEP << "\tConnect to server" << std::endl;
  
  if(SendLogon(clientSocket,cfg) != 0){
    std::cerr << "Logon failed" << std::endl;
    return 2;
  }
  std::cout << "step:2/"<< TOTAL_STEP << "\tLogon Send" << std::endl;

  if(RecvLogon(clientSocket) != 0){
    std::cerr << "Logon failed" << std::endl;
    return 3;
  }
  std::cout << "step:3/"<< TOTAL_STEP << "\tLogon Done" << std::endl;

  std::cout << "step:4/"<< TOTAL_STEP << "\tStart Receive MarketData" << std::endl;
  int bexit = 0;
  while(bexit == 0){
    bexit = RecvMsg(clientSocket);
  } 
  std::cout << "step:last/"<< TOTAL_STEP << "\tTo Exit bexit=" << bexit << std::endl;
  close(clientSocket);
  return 0;
}
