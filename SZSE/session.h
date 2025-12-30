// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#ifndef SZSE_SESSION_H
#define SZSE_SESSION_H

#include <cstdint>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iomanip>
#include "host2net.h"
#include "configuration.h"

struct v5mdhead{
  uint32_t MsgType;
  uint32_t BodyLength;
};

struct v5mdtail{
  uint32_t Checksum;
};

inline uint32_t GenerateCheckSum(char* buf, uint32_t len){
  long idx;
  uint32_t cks;
  for(idx = 0L, cks =0; idx < len; cks += (uint32_t)buf[idx++]);
  return (cks%256);
}
// #pragma pack(1)
using CompId = char[20];

struct v5mdLogonBody{
  CompId SenderCompID;
  CompId TargetCompID;
  int HeartBtInt;
  char Password[16];
  char DefaultApplVerID[32];
};

struct MsgLogon{
  v5mdhead head;
  v5mdLogonBody body;
  v5mdtail tail;
};

int SendLogon(int sock, Configuration config);

int RecvLogon(int sock);

int RecvMsg(int sock);

void OnLogon(v5mdLogonBody logon);

void OnHeartBeat();

void  OnChannelHeartBeat();

void OnRealTimeMD();

#endif
