// utilities to easier coding
//
// Created by chend on 2025/12/30.
//
#include <iostream>
#include <sys/socket.h>
#include "utility.h"

#include <iomanip>

#include "host2net.h"
#include "session.h"

int myRecv(int sock, char* buffer, size_t len) {
    size_t recvlen = 0;
    int ret = 0;
    while(recvlen < len){
        ret = recv(sock, buffer+recvlen, len-recvlen, 0);
        if(ret < 0){
            std::cerr << "receive error: " << ret << std::endl;
            return ret;
        } else if (ret == 0){
            std::cerr << "connection closed by server" << std::endl;
            return -1;
        } else {
            recvlen += ret;
        }
    }
    return ret;
}

int checkBufferLength(int headlength, int bodylength, int taillength, int bufferlength) {
    if (headlength + bodylength + taillength > bufferlength) {
        std::cerr << "large pack than expectd " << std::endl;
        return -1;
    }
    return 0;
}

int cmpCheckSum(char* buffer, uint32_t bufferLength, char* tail) {
    uint32_t checksum = GenerateCheckSum(buffer, bufferLength );
    uint32_t recvchecksum = htnu32(*(uint32_t *) tail);
    if (checksum != recvchecksum) {
        std::cerr << "Msg checksum failed\tcalc:" << std::setw(8) << std::setfill('0') << std::hex << checksum <<
            "\treceived:" << recvchecksum << std::endl;
        return -1;
    }
    return 0;
}

char* setLogonHead(void* buffer) {
    auto *p = static_cast<struct MsgLogon *>(buffer);
    p->head.MsgType = htnu32(1);
    p->head.BodyLength = htnu32(sizeof(v5mdLogonBody));
    return static_cast<char *>(buffer) + sizeof(p->head);
}

char* serializeLogonBody(const v5mdLogonBody &body, void* buffer) {
    memcpy(buffer, &body, sizeof( v5mdLogonBody));
    auto *p = static_cast<v5mdLogonBody *>(buffer);
    p->HeartBtInt = htnu32(body.HeartBtInt);
    return (static_cast<char *>(buffer) + sizeof(body));
}

char* appendTail(void *buffer, size_t length) {
    auto *p = static_cast<uint32_t *>(static_cast<void *>(static_cast<char *>(buffer) + length));
    *p = htnu32(GenerateCheckSum(static_cast<char *>(buffer), length));
    return static_cast<char *>(buffer);
}






