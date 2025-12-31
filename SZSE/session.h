// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#ifndef SZSE_SESSION_H
#define SZSE_SESSION_H

#include <cstdint>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "host2net.h"
#include "configuration.h"

struct v5mdhead{
  uint32_t MsgType;
  uint32_t BodyLength;
};

struct v5mdtail{
  uint32_t Checksum;
};


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
