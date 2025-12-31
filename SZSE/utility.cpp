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
    uint32_t *p = reinterpret_cast<uint32_t *>(static_cast<char *>(buffer) + length);
    *p = htnu32(GenerateCheckSum(static_cast<char *>(buffer), length));
    return static_cast<char *>(buffer);
}

int deserializeBody(mdData &md,const void* buffer, int length) {
    memset(&md, 0, sizeof(md));
    memcpy(&md, buffer, length);
    auto *p = static_cast<const mdData*>(buffer);
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
    if (length != md.ExtendFields.NoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup) + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md)) {
        std::cout << "length:\t" << length << std::endl;
        std::cout << "NoMDEntries:\t" << md.ExtendFields.NoMDEntries << std::endl;
        std::cout << "Expect length:\t" << md.ExtendFields.NoMDEntries * sizeof(MDEntry)+ sizeof(NumInGroup) + reinterpret_cast<char *>(&md.ExtendFields.NoMDEntries) -  reinterpret_cast<char *>(&md) << std::endl;
    }
    return 0;
}

void showMdData(const mdData & md) {
    std::cout << "OrigTime:\t" << md.OrigTime << std::endl
    << "ChannelNo:\t" << md.ChannelNo << std::endl
    << "MDStreamID:\t" << md.MDStreamID << std::endl
    << "SecurityID:\t" << md.SecurityID << std::endl
    << "SecurityIDSource:\t" << md.SecurityIDSource << std::endl
    << "TradingPhaseCode:\t" << md.TradingPhaseCode << std::endl
    << "PrevClosePx:\t" << md.PrevClosePx << std::endl
    << "NumTrades:\t" << md.NumTrades << std::endl
    << "TotalVolumeTrade:\t" << md.TotalVolumeTrade << std::endl
    << "TotalValueTrade:\t" << md.TotalValueTrade << std::endl
    << "ExtendFields:" << std::endl
    << "NoMDEntries:" << md.ExtendFields.NoMDEntries << std::endl;
    for (int i = 0; i < md.ExtendFields.NoMDEntries; i++) {
        std::cout << "Record[" << i << "]:" << std::endl
        << "MDEntryType:\t" << md.ExtendFields.MDEntryEntity[i].MDEntryType << std::endl
        << "MDEntryPx:\t" << md.ExtendFields.MDEntryEntity[i].MDEntryPx << std::endl
        << "MDEntrySize:\t" << md.ExtendFields.MDEntryEntity[i].MDEntrySize<< std::endl
        << "MDPriceLevel:\t" <<md.ExtendFields.MDEntryEntity[i].MDPriceLevel<< std::endl
        << "NumberOfOrders:\t" <<md.ExtendFields.MDEntryEntity[i].NumberOfOrders<< std::endl
        << "NoOrders:\t" <<md.ExtendFields.MDEntryEntity[i].NoOrders<< std::endl;
    }
}


