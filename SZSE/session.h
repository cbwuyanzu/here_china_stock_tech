// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#ifndef SZSE_SESSION_H
#define SZSE_SESSION_H

#include "types.h"

int SendLogon(int sock, ReqLogon config);

int RecvLogon(int sock);

int RecvMsg(int sock);

void OnLogon(v5mdLogonBody logon);

void OnHeartBeat();

void  OnChannelHeartBeat();

void OnRealTimeMD(void* data, int length);

#endif
