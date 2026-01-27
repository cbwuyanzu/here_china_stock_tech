// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#ifndef SZSE_SESSION_H
#define SZSE_SESSION_H

#include "myFIFOQueue.h"
#include "myFIFOQueueNL.h"
#include "types.h"

int SendLogon(int sock, ReqLogonCfg config);
int SendLogout(int sock);

int RecvLogon(int sock);
int RecvLogout(int sock);

int RecvMsg(int sock);

void OnLogon(v5mdLogonBody logon);
void OnLogout(v5mdLogoutBody logon);

void OnHeartBeat();

void OnChannelHeartBeat();

char* setLogonHead(void* buffer);
char* setLogoutHead(void* buffer);

char* serializeLogonBody(const v5mdLogonBody &body, void* buffer);
char* serializeLogoutBody(const v5mdLogoutBody &body, void* buffer);


#endif
