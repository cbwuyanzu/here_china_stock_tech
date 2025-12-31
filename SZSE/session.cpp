// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

#include "utility.h"

int SendLogon(int sock, Configuration config) {
    // MsgLogon msg = {};
    v5mdLogonBody body = {};
    strcpy(body.SenderCompID, config.szLocalName);
    strcpy(body.TargetCompID, config.szTargetName);
    body.HeartBtInt = config.iHeartBeat;
    strcpy(body.Password, config.szPassword);
    strcpy(body.DefaultApplVerID, config.szVersion);
    char buf[1024] = {};
    char* posbody = setLogonHead(buf);
    char* postail = serializeLogonBody(body,posbody);
    appendTail(postail,sizeof(v5mdhead)+sizeof(v5mdLogonBody));
    uint32_t msglen = sizeof(MsgLogon);
    if (send(sock, buf, msglen, 0) == -1) {
        std::cerr << "Message send failed" << std::endl;
        return -1;
    } else {
        std::cout << "Message sent logon" << std::endl;
    }
    return 0;
}

int RecvLogon(int sock) {
    char buffer[1024] = {0};
    int ret = 0;
    ret = myRecv(sock, buffer, sizeof(v5mdhead));
    if (ret <= 0) return -1;
    v5mdhead *hd = (v5mdhead *) buffer;
    std::cout << "Logon response msgtype\t" << htnu32(hd->MsgType) << std::endl;
    std::cout << "Logon response datalen\t" << htnu32(hd->BodyLength) << std::endl;
    uint32_t msgType = htnu32(hd->MsgType);
    uint32_t bodyLen = htnu32(hd->BodyLength);
    if (msgType != 1 || bodyLen != sizeof(v5mdLogonBody)) {
        std::cerr << "Logon Response MsgType:[" << msgType << "] BodyLength:[" << bodyLen << "] expected:[1][" << sizeof
                (v5mdLogonBody) << "]" << std::endl;
        return -1;
    }
    char *body = (char *) (hd + 1);
    char *tail = (char *) (body + bodyLen);
    ret = myRecv(sock,body,bodyLen + sizeof(v5mdtail));
    if (ret <= 0) return -2;
    ret = cmpCheckSum(buffer,sizeof(v5mdhead) + bodyLen, tail);
    if (ret == 0) {
        std::cout << "Logon checksum passed" << std::endl;
    } else {
        std::cerr << "Logon checksum failed" << std::endl;
        return -1;
    }
    v5mdLogonBody logon = *(v5mdLogonBody *) body;
    OnLogon(logon);
    return 0;
}

int RecvMsg(int sock) {
    char buffer[4096] = {0};
    int ret = 0;
    ret = myRecv(sock, buffer, sizeof(v5mdhead));
    if (ret <= 0) {
        return -1;
    }
    v5mdhead *hd = (v5mdhead *) buffer;
    uint32_t msgtype = htnu32(hd->MsgType);
    uint32_t bodylength = htnu32(hd->BodyLength);
    char *body = (char *) (hd + 1);
    char *tail = (char *) (body + bodylength);
    ret = myRecv(sock, body, bodylength + sizeof(v5mdtail));
    if (ret <= 0) {
        return -2;
    }
    ret = checkBufferLength(sizeof(v5mdhead),bodylength,sizeof(v5mdtail),sizeof(buffer));
    if (ret < 0) {
        std::cout << "Msg received msgtype\t" << msgtype << std::endl;
        std::cout << "Msg received datalen\t" << bodylength << std::endl;
        return -3;
    }
    ret = cmpCheckSum(buffer, sizeof(v5mdhead) + bodylength,tail);
    if (ret < 0) {
        std::cerr << "Last Msg received msgtype\t" << std::dec << msgtype << std::endl;
        std::cerr << "Last Msg received datalen\t" << bodylength << std::endl;
        return -4;
    }
    //TODO 这里还需要再加一些统计次数和耗时
    switch (msgtype) {
        case 3:
            OnHeartBeat();
            break;
        case 390095:
            OnChannelHeartBeat();
            break;
        case 300111:
            OnRealTimeMD(body, bodylength);
            break;
    }
    return 0;
}

void OnLogon(const v5mdLogonBody logon) {
    std::cout << "SenderCompID\t" << logon.SenderCompID << std::endl;
    std::cout << "TargetCompID\t" << logon.TargetCompID << std::endl;
    std::cout << "HeartBtInt\t" << htn32(logon.HeartBtInt) << std::endl;
    std::cout << "Password\t" << logon.Password << std::endl;
    std::cout << "DefaultApplVerID\t" << logon.DefaultApplVerID << std::endl;
}

uint32_t hbcount = 0;

void OnHeartBeat() {
    hbcount++;
    if (hbcount % 100 == 0)
        std::cout << "HeartBeatCount\t" << hbcount << std::endl;
}

uint32_t chhb = 0;

void OnChannelHeartBeat() {
    chhb++;
    if (chhb % 100 == 0)
        std::cout << "ChannelHeartBeatCount\t" << chhb << std::endl;
}

uint32_t rtmdcount = 0;

void OnRealTimeMD(void* data,int length) {
    rtmdcount++;
    if (rtmdcount % 100 == 0)
        std::cout << "RealTimeMDCount\t" << rtmdcount << std::endl;
    mdData md = {};
    deserializeBody(md,data,length);
    showMdData(md);
}
/*Standard Header 消息头
MsgType=3xxx11
OrigTime 数据生成时间
ChannelNo 频道代码
MDStreamID 行情类别
SecurityID 证券代码
SecurityIDSource 证券代码源
TradingPhaseCode 产品所处的交易阶段代码
PrevClosePx 昨收价
NumTrades 成交笔数
TotalVolumeTrade 成交总量
TotalValueTrade 成交总金额
Extend Fields 各业务扩展字段*/