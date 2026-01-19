// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#ifndef SZSE_SESSION_H
#define SZSE_SESSION_H

#include "myFIFOQueue.h"
#include "types.h"

int SendLogon(int sock, ReqLogonCfg config);

int RecvLogon(int sock);

int RecvMsg(int sock);

void OnLogon(v5mdLogonBody logon);

void OnHeartBeat();

void OnChannelHeartBeat();

char* setLogonHead(void* buffer);

char* serializeLogonBody(const v5mdLogonBody &body, void* buffer);


#endif
