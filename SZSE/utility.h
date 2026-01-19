// utilities to easier coding
//
// Created by chend on 2025/12/30.
//

#ifndef SZSE_UTILITY_H
#define SZSE_UTILITY_H

#include "types.h"

#define bswap16(x) (((x & 0xff00) >> 8 )| ((x & 0x00ff) << 8))
#define bswap32(x) (((x & 0xff000000) >> 24 )| ((x & 0x000000ff) << 24) | \
((x & 0x00ff0000) >>  8 )| ((x & 0x0000ff00) <<  8))
#define bswap64(x) (((x & 0xff00000000000000) >> 56 )| ((x & 0x00000000000000ff) << 56) | \
((x & 0x00ff000000000000) >> 40 )| ((x & 0x000000000000ff00) << 40) | \
((x & 0x0000ff0000000000) >> 24 )| ((x & 0x0000000000ff0000) << 24) | \
((x & 0x000000ff00000000) >>  8 )| ((x & 0x00000000ff000000) <<  8))

inline uint16_t htnu16(uint16_t x){
    uint16_t res = bswap16(x);
    return res;
}

inline uint32_t htnu32(uint32_t x){
    uint32_t res = bswap32(x);
    return res;
}

inline uint64_t htnu64(uint64_t x){
    uint64_t res = bswap64(x);
    return res;
}

inline int16_t htn16(int16_t x){
    int16_t res = bswap16(x);
    return res;
}

inline int32_t htn32(int32_t x){
    int32_t res = bswap32(x);
    return res;
}

inline int64_t htn64(int64_t x){
    int64_t res = bswap64(x);
    return res;
}

int myConnect(const char* serverIP,const int port, int &sock_fd);

int myClose(int sock_fd);

int myRecv(int sock, char* buffer, std::size_t len);

int checkBufferLength(uint32_t headlength, uint32_t bodylength, uint32_t taillength, uint32_t bufferlength);

int cmpCheckSum(char* buffer, uint32_t bufferLength, char* tail);

inline uint32_t GenerateCheckSum(char* buf, uint32_t len){
    long idx;
    uint32_t cks;
    for(idx = 0L, cks =0; idx < len; cks += (uint32_t)buf[idx++]);
    return (cks%256);
}

char* appendTail(void *buffer, std::size_t length);

int fixedCharToInt(const char* str, std::size_t len);

long long getTimestampAsLongLong();

void printCmd();

void show();

void dump();

#endif //SZSE_UTILITY_H
