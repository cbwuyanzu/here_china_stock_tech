// utilities to easier coding
//
// Created by chend on 2025/12/30.
//
//sockaddr_in AF_INET connect sockaddr inet_addr
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utility.h"
#include "host2net.h"
#include "loggerSingleton.h"

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

int checkBufferLength(int headlength, int bodylength, int taillength, int bufferlength) {
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
    uint32_t *p = reinterpret_cast<uint32_t *>(static_cast<char *>(buffer) + length);
    *p = htnu32(GenerateCheckSum(static_cast<char *>(buffer), length));
    return static_cast<char *>(buffer);
}

int deserializeBody(RawSzMDData &md,const void* buffer, int length) {
    auto *p = static_cast<const RawSzMDData*>(buffer);
    //这里先做一些包的检查, 以免引起coredump
    uint32_t uNoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    if ( uNoMDEntries > MAX_MD_ENTRY_NO) {
        return -1;
    }
    if (length != uNoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup) + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md)) {
        LOG_ERROR("NoMDEntries:\t{}\nlength:\t{}\nExpect length:{}",
            md.ExtendFields.NoMDEntries, length,
            md.ExtendFields.NoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup)
            + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md));
        return -2;
    }
    memset(&md, 0, sizeof(md));
    memcpy(&md, buffer, length);
    md.OrigTime = htn64(p->OrigTime);
    md.ChannelNo = htnu16(p->ChannelNo);
    md.PrevClosePx = htn64(p->PrevClosePx);
    md.NumTrades = htn64(p->NumTrades);
    md.TotalVolumeTrade = htn64(p->TotalVolumeTrade);
    md.TotalValueTrade = htn64(p->TotalValueTrade);
    md.ExtendFields.NoMDEntries = htnu32(p->ExtendFields.NoMDEntries);
    for (int i = 0; i < md.ExtendFields.NoMDEntries; i++) {
        md.ExtendFields.MDEntryEntity[i].MDEntryPx = htn64(p->ExtendFields.MDEntryEntity[i].MDEntryPx);
        md.ExtendFields.MDEntryEntity[i].MDEntrySize = htn64(p->ExtendFields.MDEntryEntity[i].MDEntrySize);
        md.ExtendFields.MDEntryEntity[i].MDPriceLevel = htnu16(p->ExtendFields.MDEntryEntity[i].MDPriceLevel);
        md.ExtendFields.MDEntryEntity[i].NumberOfOrders = htn64(p->ExtendFields.MDEntryEntity[i].NumberOfOrders);
        md.ExtendFields.MDEntryEntity[i].NoOrders = htnu32(p->ExtendFields.MDEntryEntity[i].NoOrders);
    }
    return 0;
}

void showMdData(const RawSzMDData & md) {
    LOG_DEBUG("OrigTime:{}",md.OrigTime);
    LOG_DEBUG("ChannelNo:{}",md.ChannelNo);
    LOG_DEBUG("MDStreamID:{:.{}}",md.MDStreamID, sizeof(md.MDStreamID));
    LOG_DEBUG("SecurityID:{:.{}}",md.SecurityID, sizeof(md.SecurityID));
    LOG_DEBUG("SecurityIDSource:{:.{}}",md.SecurityIDSource, sizeof(md.SecurityIDSource));
    LOG_DEBUG("TradingPhaseCode:{:.{}}",md.TradingPhaseCode, sizeof(md.TradingPhaseCode));
    LOG_DEBUG("PrevClosePx:{}",md.PrevClosePx);
    LOG_DEBUG("NumTrades:{}",md.NumTrades);
    LOG_DEBUG("TotalVolumeTrade:{}",md.TotalVolumeTrade);
    LOG_DEBUG("TotalValueTrade:{}",md.TotalValueTrade);
    LOG_DEBUG("ExtendFields:");
    LOG_DEBUG("NoMDEntries:{}",md.ExtendFields.NoMDEntries)
    for (int i = 0; i < md.ExtendFields.NoMDEntries; i++) {
        LOG_DEBUG("Record[{}]:",i);
        LOG_DEBUG("MDEntryType:{:.{}}",md.ExtendFields.MDEntryEntity[i].MDEntryType,sizeof(md.ExtendFields.MDEntryEntity[i].MDEntryType));
        LOG_DEBUG("MDEntryPx:{}",md.ExtendFields.MDEntryEntity[i].MDEntryPx);
        LOG_DEBUG("MDEntrySize:{}",md.ExtendFields.MDEntryEntity[i].MDEntrySize);
        LOG_DEBUG("MDPriceLevel:{}",md.ExtendFields.MDEntryEntity[i].MDPriceLevel);
        LOG_DEBUG("NumberOfOrders:{}",md.ExtendFields.MDEntryEntity[i].NumberOfOrders);
        LOG_DEBUG("NoOrders:{}",md.ExtendFields.MDEntryEntity[i].NoOrders);
    }
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
    printf("  q/Q      quit\n");
    printf("  q/Q      quit\n");
    printf("  q/Q      quit\n");
}