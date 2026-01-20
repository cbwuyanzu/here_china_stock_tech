// utilities to easier coding
//
// Created by chend on 2025/12/30.
//
//sockaddr_in AF_INET connect sockaddr inet_addr

#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include "loggerSingleton.h"
#include "szMdParser.h"

extern SZMDParser szMDParser;

int myConnect(const char* serverIP,const int port, int &sock_fd) {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        LOG_CRITICAL("Socket creation failed");
        return 1;
    }
    sock_fd = clientSocket;
    // 设置服务器地址
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    if (connect(clientSocket, (sockaddr *) &serverAddr, sizeof(serverAddr)) == -1) {
        LOG_CRITICAL("Connection failed");
        close(clientSocket);
        return 1;
    }
    return 0;
}

int myClose(int sock_fd) {
    close(sock_fd);
    return 0;
}

int myRecv(int sock, char* buffer, size_t len) {
    size_t recvlen = 0;
    int ret = 0;
    while(recvlen < len){
        ret = recv(sock, buffer+recvlen, len-recvlen, 0);
        if(ret < 0){
            LOG_ERROR("receive error: {}", ret);
            return ret;
        } else if (ret == 0){
            LOG_ERROR("connection closed by server");
            return -1;
        } else {
            recvlen += ret;
        }
    }
    return ret;
}

int checkBufferLength(uint32_t headlength, uint32_t bodylength, uint32_t taillength, uint32_t bufferlength) {
    if (headlength + bodylength + taillength > bufferlength) {
        LOG_ERROR("large pack than expected");
        return -1;
    }
    return 0;
}

int cmpCheckSum(char* buffer, uint32_t bufferLength, char* tail) {
    uint32_t checksum = GenerateCheckSum(buffer, bufferLength );
    uint32_t recvchecksum = htnu32(*(uint32_t *) tail);
    if (checksum != recvchecksum) {
        LOG_ERROR("Msg checksum failed\tcalc:{:x}\treceived:{:x}",checksum,recvchecksum);
        return -1;
    }
    return 0;
}

char* appendTail(void *buffer, size_t length) {
    uint32_t *p = reinterpret_cast<uint32_t *>(static_cast<char *>(buffer) + length);
    *p = htnu32(GenerateCheckSum(static_cast<char *>(buffer), length));
    return static_cast<char *>(buffer);
}


int fixedCharToInt(const char* str, std::size_t len) {
    int result = 0;
    for (std::size_t i = 0; i < len; ++i) {
        // 确保是数字字符
        if (str[i] < '0' || str[i] > '9') {
            // 简单处理：遇到非数字停止
            // 也可选择抛出异常或返回特定错误码
            break;
        }
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

long long getTimestampAsLongLong() {
    using namespace std::chrono;
    // 获取当前时间（毫秒精度）
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch());
    time_t sec = ms.count() / 1000;
    int millis = ms.count() % 1000;
    // 获取本地时间结构体（线程安全版本）
    struct tm timeinfo;
    localtime_r(&sec, &timeinfo);
    // 直接计算long long值（避免字符串转换）
    long long result = (timeinfo.tm_year + 1900LL) * 10000000000000LL; // 年
    result += (timeinfo.tm_mon + 1) * 100000000000LL;                 // 月
    result += timeinfo.tm_mday * 1000000000LL;                        // 日
    result += timeinfo.tm_hour * 10000000LL;                          // 时
    result += timeinfo.tm_min * 100000LL;                             // 分
    result += timeinfo.tm_sec * 1000LL;                               // 秒
    result += millis;                                                 // 毫秒
    return result;
}

void printCmd() {
    printf("press your cmd:\n");
    printf("  Q      Quit\n");
    printf("  S      Show\n");
    printf("  D      Dump\n");
}

void show() {
    szMDParser.show();
    std::cout << "Connect Status: not finished yet" << std::endl;
}

void dump() {
    szMDParser.dump("dump.txt");
}