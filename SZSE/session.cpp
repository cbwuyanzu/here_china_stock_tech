// class to manage session with MDGW
//
// Created by dzg on 2025/12/30.
//

//send
#include <netinet/in.h>
#include "loggerSingleton.h"
#include "utility.h"
#include "session.h"

int SendLogon(int sock, ReqLogonCfg reqLogon) {
    // MsgReqLogon msg = {};
    v5mdLogonBody body = {};
    strcpy(body.SenderCompID, reqLogon.szLocalName);
    strcpy(body.TargetCompID, reqLogon.szTargetName);
    body.HeartBtInt = reqLogon.iHeartBeat;
    strcpy(body.Password, reqLogon.szPassword);
    strcpy(body.DefaultApplVerID, reqLogon.szVersion);
    char buf[1024] = {};
    char* posbody = setLogonHead(buf);
    char* postail = serializeLogonBody(body,posbody);
    appendTail(postail,sizeof(v5MDHead)+sizeof(v5mdLogonBody));
    uint32_t msglen = sizeof(MsgReqLogon);
    if (send(sock, buf, msglen, 0) == -1) {
        LOG_ERROR("Message send failed");
        return -1;
    } else {
        LOG_INFO("Message sent logon");
    }
    return 0;
}

int RecvLogon(int sock) {
    char buffer[1024] = {0};
    int ret = 0;
    ret = myRecv(sock, buffer, sizeof(v5MDHead));
    if (ret <= 0) return -1;
    auto *hd = reinterpret_cast<v5MDHead *>(buffer);
    LOG_INFO("Logon response msgtype\t{}",htnu32(hd->MsgType));
    LOG_INFO("Logon response datalen\t{}",htnu32(hd->BodyLength));
    uint32_t msgType = htnu32(hd->MsgType);
    uint32_t bodyLen = htnu32(hd->BodyLength);
    if (msgType != 1 || bodyLen != sizeof(v5mdLogonBody)) {
        LOG_ERROR("Logon Response MsgType:[{}] BodyLength:[{}] expected:[1][{}]",
            msgType, bodyLen, sizeof(v5mdLogonBody));
        return -1;
    }
    char *body = (char *) (hd + 1);
    char *tail = (char *) (body + bodyLen);
    ret = myRecv(sock,body,bodyLen + sizeof(v5MDTail));
    if (ret <= 0) return -2;
    ret = cmpCheckSum(buffer,sizeof(v5MDHead) + bodyLen, tail);
    if (ret == 0) {
        LOG_INFO("Logon checksum passed");
    } else {
        LOG_ERROR("Logon checksum failed");
        return -1;
    }
    v5mdLogonBody logon = *(v5mdLogonBody *) body;
    OnLogon(logon);
    return 0;
}

int SendLogout(int sock) {
    v5mdLogoutBody body = {};
    body.session_status = 0;
    strcpy(body.text,"Logout");
    char buf[1024] = {};
    char* posbody = setLogoutHead(buf);
    char* postail = serializeLogoutBody(body,posbody);
    appendTail(postail,sizeof(v5MDHead)+sizeof(v5mdLogoutBody));
    uint32_t msglen = sizeof(MsgReqLogout);
    if (send(sock, buf, msglen, 0) == -1) {
        LOG_ERROR("Message send failed");
        return -1;
    } else {
        LOG_INFO("Message sent logout");
    }
    return 0;
}

int RecvLogout(int sock) {
    char buffer[1024] = {0};
    int ret = 0;
    ret = myRecv(sock, buffer, sizeof(v5MDHead));
    if (ret <= 0) return -1;
    auto *hd = reinterpret_cast<v5MDHead *>(buffer);
    LOG_INFO("Logout response msgtype\t{}",htnu32(hd->MsgType));
    LOG_INFO("Logout response datalen\t{}",htnu32(hd->BodyLength));
    uint32_t msgType = htnu32(hd->MsgType);
    uint32_t bodyLen = htnu32(hd->BodyLength);
    if (msgType != 2 || bodyLen != sizeof(v5mdLogoutBody)) {
        LOG_ERROR("Logout Response MsgType:[{}] BodyLength:[{}] expected:[1][{}]",
            msgType, bodyLen, sizeof(v5mdLogoutBody));
        return -1;
    }
    char *body = (char *) (hd + 1);
    char *tail = (char *) (body + bodyLen);
    ret = myRecv(sock,body,bodyLen + sizeof(v5MDTail));
    if (ret <= 0) return -2;
    ret = cmpCheckSum(buffer,sizeof(v5MDHead) + bodyLen, tail);
    if (ret == 0) {
        LOG_INFO("Logout checksum passed");
    } else {
        LOG_ERROR("Logout checksum failed");
        return -1;
    }
    v5mdLogoutBody logout = *(v5mdLogoutBody *) body;
    OnLogout(logout);
    return 0;
}


int RecvMsg(int sock) {
    char buffer[4096] = {};
    int ret = 0;
    ret = myRecv(sock, buffer, sizeof(v5MDHead));
    if (ret <= 0) {
        return -1;
    }
    auto *hd = reinterpret_cast<v5MDHead *>(buffer);
    hd->MsgType = htnu32(hd->MsgType);
    hd->BodyLength = htnu32(hd->BodyLength);
    uint32_t msgType = hd->MsgType;
    uint32_t BodyLength = hd->BodyLength;
    char *body = reinterpret_cast<char *>(hd + 1);
    char *tail = body + BodyLength;
    ret = myRecv(sock, body, BodyLength + sizeof(v5MDTail));
    if (ret <= 0) {
        return -2;
    }
    ret = checkBufferLength(sizeof(v5MDHead),BodyLength,sizeof(v5MDTail),sizeof(buffer));
    if (ret < 0) {
        LOG_INFO("Msg received msgType\t{}", msgType);
        LOG_INFO("Msg received BodyLength\t{}", BodyLength);
        return -3;
    }
    ret = cmpCheckSum(buffer, sizeof(v5MDHead) + BodyLength,tail);
    if (ret < 0) {
        LOG_ERROR("Last Msg received msgType\t{}",msgType);
        LOG_ERROR("Last Msg received BodyLength\t{}",BodyLength);
        return -4;
    }
    //TODO 这里还需要再加一些统计次数和耗时

    auto start = std::chrono::high_resolution_clock::now();
    switch (hd->MsgType) {
        case 3:
            OnHeartBeat();
            break;
        case 390095:
            OnChannelHeartBeat();
            break;
        //会话管理类功能要同步处理

        //业务请求转到异步线程做吧
        default:
            v5QueueData queueData{};
            memcpy(&queueData,buffer,sizeof(v5MDHead)+BodyLength);
            APPINSTANCE.getQueue().push(queueData);
            break;
    }
    {
        APPINSTANCE.getIoMutex().lock();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        auto it = APPINSTANCE.getIoFs().find(msgType);
        if ( it == APPINSTANCE.getIoFs().end()) {
            APPINSTANCE.getIoFs().insert(std::make_pair(msgType,funcStat{1,0,duration.count()}));
        } else {
            it->second.success++;
            it->second.successTimeCostUs += duration.count();
        }
        APPINSTANCE.getIoMutex().unlock();
    }
    return 0;
}

void OnLogon(const v5mdLogonBody logon) {
    LOG_INFO("SenderCompID:{}",logon.SenderCompID);
    LOG_INFO("TargetCompID:{}", logon.TargetCompID);
    LOG_INFO("HeartBtInt:{}",htn32(logon.HeartBtInt));
    LOG_INFO("Password:{:.{}}", logon.Password, sizeof(logon.Password));
    LOG_INFO("DefaultApplVerID:{:.{}}", logon.DefaultApplVerID, sizeof(logon.DefaultApplVerID));
}

void OnLogout(const v5mdLogoutBody logout) {
    LOG_INFO("session_status:{}",htn32(logout.session_status));
    LOG_INFO("text:{}", logout.text);
}

uint32_t hbcount = 0;
void OnHeartBeat() {
    hbcount++;
    if (hbcount % 100 == 0)
        LOG_INFO("HeartBeatCount\t{}",hbcount);
}

uint32_t chhb = 0;
void OnChannelHeartBeat() {
    chhb++;
    if (chhb % 100 == 0)
        LOG_INFO("ChannelHeartBeatCount\t{}",chhb);
}

char* setLogonHead(void* buffer) {
    auto *p = static_cast<struct MsgReqLogon *>(buffer);
    p->head.MsgType = htnu32(1);
    p->head.BodyLength = htnu32(sizeof(v5mdLogonBody));
    return static_cast<char *>(buffer) + sizeof(p->head);
}

char* setLogoutHead(void* buffer) {
    auto *p = static_cast<struct MsgReqLogout *>(buffer);
    p->head.MsgType = htnu32(2);
    p->head.BodyLength = htnu32(sizeof(v5mdLogoutBody));
    return static_cast<char *>(buffer) + sizeof(p->head);
}

char* serializeLogonBody(const v5mdLogonBody &body, void* buffer) {
    memcpy(buffer, &body, sizeof( v5mdLogonBody));
    auto *p = static_cast<v5mdLogonBody *>(buffer);
    p->HeartBtInt = htnu32(body.HeartBtInt);
    return (static_cast<char *>(buffer) + sizeof(body));
}

char* serializeLogoutBody(const v5mdLogoutBody &body, void* buffer) {
    memcpy(buffer, &body, sizeof( v5mdLogoutBody));
    auto *p = static_cast<v5mdLogoutBody *>(buffer);
    p->session_status = htn32(body.session_status);
    return (static_cast<char *>(buffer) + sizeof(body));
}