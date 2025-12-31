// utilities to easier coding
//
// Created by chend on 2025/12/30.
//

#ifndef SZSE_UTILITY_H
#define SZSE_UTILITY_H
#include <cstdint>
#include "session.h"


int myRecv(int sock, char* buffer, size_t len);

int checkBufferLength(int headlength, int bodylength, int taillength, int bufferlength);

int cmpCheckSum(char* buffer, uint32_t bufferLength, char* tail);

inline uint32_t GenerateCheckSum(char* buf, uint32_t len){
    long idx;
    uint32_t cks;
    for(idx = 0L, cks =0; idx < len; cks += (uint32_t)buf[idx++]);
    return (cks%256);
}

char* setLogonHead(void* buffer);

char* serializeLogonBody(const struct v5mdLogonBody &body, void* buffer);

char* appendTail(void *buffer, size_t length);

int deserializeBody(mdData &md,const void* buffer, int length);

void showMdData(const mdData & md);

#endif //SZSE_UTILITY_H
