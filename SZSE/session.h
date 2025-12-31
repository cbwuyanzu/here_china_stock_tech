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
#pragma pack(1)
//二进制解包 必须字节对齐

struct v5mdhead{
  uint32_t MsgType;
  uint32_t BodyLength;
};

struct v5mdtail{
  uint32_t Checksum;
};

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

void OnRealTimeMD(void* data, int length);

using NumInGroup = uint32_t;

using LocalTimeStamp = int64_t;
using SecurityIDType = char[8];
using Price = int64_t;
using Qty = int64_t;
using Amt = int64_t;

struct MDEntry{
  char MDEntryType[2];
  int64_t MDEntryPx;
  Qty MDEntrySize;
  uint16_t MDPriceLevel;
  int64_t NumberOfOrders;
  NumInGroup NoOrders;
  // Qty OrderQty;
};

//union ExtendFieldType {
  struct ExtendFieldType {
    NumInGroup NoMDEntries{};
    MDEntry MDEntryEntity[20]{};
  };
// };

struct mdData {
  LocalTimeStamp OrigTime;
  uint16_t ChannelNo;
  char MDStreamID[3];
  SecurityIDType SecurityID;
  char SecurityIDSource[4];
  char TradingPhaseCode[8];
  Price PrevClosePx;
  int64_t NumTrades;
  Qty TotalVolumeTrade;
  Amt TotalValueTrade;
  ExtendFieldType ExtendFields;
};

#endif
