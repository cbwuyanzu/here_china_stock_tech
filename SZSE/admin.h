#include <stdint.h>
#include "host2net.h"

struct v5mdhead{
  uint32_t MsgType;
  uint32_t BodyLength;
};

struct v5mdtail{
  uint32_t Checksum;
};

inline uint32_t GenerateCheckSum(char* buf, uint32_t len){
  long idx;
  uint32_t cks;
  for(idx = 0L, cks =0; idx < len; cks += (uint32_t)buf[idx++]);
  return (cks%256);
}
#pragma pack(1)
using CompId = char[20];

struct v5mdLogonBody{
  CompId SenderCompID;
  CompId TargetCompID;
  int HeartBtInt;
  char Password[16];
  char DefaultApplVerID[32];
};

struct MsgLogon{
  v5mdhead head;
  v5mdLogonBody body;
  v5mdtail tail;
};

#define SENDER "MYSENDER"
#define RECEIVER "MYRECEIVER"
#define PASSWD "MYPASSWD"
#define VERSION "1.0.0"

int SendLogon(int sock){
  MsgLogon msg = {};
  msg.head.MsgType = htnu32(1);
  msg.head.BodyLength = htnu32(sizeof(v5mdLogonBody));
  v5mdLogonBody body ={SENDER,RECEIVER,htn32(30),PASSWD, VERSION};
  msg.body = body;
  char* buf = (char*) &msg;
  uint32_t buflen = sizeof(v5mdhead) + sizeof(v5mdLogonBody);
  uint32_t msglen = sizeof(msg);
  msg.tail.Checksum = htnu32(GenerateCheckSum(buf,buflen));
  if (send(sock, buf, msglen, 0) == -1) {
    std::cerr << "Message send failed" << std::endl;
    return -1;
  } else {
    std::cout << "Message sent logon"   << std::endl;
  }
  return 0;
}

void OnLogon(v5mdLogonBody logon);

int RecvLogon(int sock){
  char buffer[1024] = {0};
  int ret = 0;
  uint32_t recvlen = 0;
  while(recvlen < sizeof(v5mdhead)){
    ret = recv(sock, buffer+recvlen, sizeof(v5mdhead)-recvlen, 0);
    if(ret < 0){
      std::cerr << "Receive head failed" << std::endl;
      return ret;
    } else if (ret == 0){
      std::cout << "Connection closed by server receiving head" << std::endl;
      return -1;
    } else {
      recvlen += ret;
      std::cout << "Logon response head" << std::endl;
    }
  }
  v5mdhead* hd = (v5mdhead*) buffer;
  std::cout << "Logon response msgtype\t" << htnu32(hd->MsgType) <<std::endl;
  std::cout << "Logon response datalen\t" << htnu32(hd->BodyLength) <<std::endl;
  uint32_t msgType = htnu32(hd->MsgType);
  uint32_t bodyLen = htnu32(hd->BodyLength);
  if(msgType != 1|| bodyLen != sizeof(v5mdLogonBody)){
    std::cerr << "Logon Response MsgType:[" << msgType << "] BodyLength:[" << bodyLen<<"] expected:[1][" << sizeof(v5mdLogonBody) <<"]" << std::endl;
    return -1;
  }
  char *body = (char*) (hd + 1);
  char *tail = (char*) (body + bodyLen);
  ret = 0;
  recvlen = 0;
  while(recvlen < bodyLen + sizeof(v5mdtail)){
    ret = recv(sock, body+recvlen, bodyLen+sizeof(v5mdtail)-recvlen, 0);
    if(ret < 0){
      std::cerr << "Receive body failed" << std::endl;
      return ret;
    } else if (ret == 0){
      std::cout << "Connection closed by server receiving body" << std::endl;
      return -1;
    } else {
      recvlen += ret;
    }
  }
  uint32_t checksum = GenerateCheckSum(buffer,sizeof(v5mdhead)+bodyLen);
  if(htnu32(checksum) == *(uint32_t*) tail){
    std::cout << "Logon checksum passed" << std::endl;
  }
  else {
    std::cerr << "Logon checksum failed" << std::endl;
    return -1;
  }
  v5mdLogonBody logon = * (v5mdLogonBody*) body;
  OnLogon(logon);
  return 0;
}

void OnLogon(const v5mdLogonBody logon){
  std::cout <<  "SenderCompID\t" << logon.SenderCompID<< std::endl;
  std::cout <<  "TargetCompID\t" << logon.TargetCompID<< std::endl;
  std::cout <<  "HeartBtInt\t" << htn32(logon.HeartBtInt)<< std::endl;
  std::cout <<  "Password\t" << logon.Password<< std::endl;
  std::cout <<  "DefaultApplVerID\t" << logon.DefaultApplVerID << std::endl;
}
uint32_t hbcount = 0;
void OnHeartBeat(){
  hbcount++;
  if(hbcount % 100 == 0)
    std::cout <<  "HeartBeatCount\t" << hbcount<< std::endl;
}
uint32_t chhb = 0;
void  OnChannelHeartBeat(){
  chhb++;
  if(chhb % 100 == 0)
    std::cout <<  "ChannelHeartBeatCount\t" << chhb<< std::endl;
}

uint32_t rtmdcount = 0;
void OnRealTimeMD(){
  rtmdcount++;
  if(rtmdcount % 100 == 0)
    std::cout <<  "RealTimeMDCount\t" << rtmdcount << std::endl;
}

int RecvMsg(int sock){
  char buffer[4096] = {0};
  int ret = 0;
  uint32_t recvlen = 0;
  while(recvlen < sizeof(v5mdhead)){
    ret = recv(sock, buffer+recvlen, sizeof(v5mdhead)-recvlen, 0);
    if (ret < 0) {
      std::cerr << "Receive head failed" << std::endl;
      return ret;
    } else if (ret == 0) {
      std::cout << "Connection closed by server receiving head" << std::endl;
      return -1;
    } else {
      recvlen+=ret;
    }
  }
  v5mdhead* hd = (v5mdhead*) buffer;
  uint32_t msgtype = htnu32(hd->MsgType);
  uint32_t bodylength= htnu32(hd->BodyLength);
  std::cout << "Msg received msgtype\t" << msgtype <<std::endl;
  std::cout << "Msg received datalen\t" << bodylength <<std::endl;
  char *body = (char*) (hd + 1);
  char *tail = (char*) (body + bodylength);
  ret = 0;
  recvlen = 0;
  while(recvlen < bodylength + sizeof(v5mdtail))
  {
    ret = recv(sock, body+recvlen, bodylength+sizeof(v5mdtail)-recvlen, 0);
    if(ret < 0){
      std::cerr << "Receive body failed" << std::endl;
      return ret;
    } else if (ret == 0){
      std::cout << "Connection closed by server receiving body" << std::endl;
      return -1;
    } else {
      recvlen += ret;
      std::cout << "Received length:[cur_loop|accu_loop|expect]"<< ret <<"|"<< recvlen << "|" << bodylength+sizeof(v5mdtail)<< std::endl;
    }
  }
  if(bodylength+sizeof(v5mdhead)+sizeof(v5mdtail) >= 4096) {
    std::cout << "large pack than expectd " <<std::endl;
    std::cout << "Msg received msgtype\t" << msgtype <<std::endl;
    std::cout << "Msg received datalen\t" << bodylength <<std::endl;
    return -1;
  }
  uint32_t checksum = GenerateCheckSum(buffer,sizeof(v5mdhead)+bodylength);
  uint32_t recvchecksum = htnu32(*(uint32_t*)tail);
  if(checksum != recvchecksum){
    std::cerr << "Msg checksum failed\tcalc:" << std::setw(8) << std::setfill('0') << std::hex << checksum<< "\treceived:"<< recvchecksum << std::endl;
    std::cerr << "Last Msg received msgtype\t" << std::dec << msgtype <<std::endl;
    std::cerr << "Last Msg received datalen\t" << bodylength <<std::endl;
    std::cerr << "Received length:[cur_loop|accu_loop|expect]"<< ret <<"|"<< recvlen << "|" << bodylength+sizeof(v5mdtail)<< std::endl;
    return -1;
  }
  switch (msgtype){
    case 3:
      OnHeartBeat();
      break;
    case 390095:
      OnChannelHeartBeat();
      break;
    case 300111:
      OnRealTimeMD();
      break;
  }
  return 0;
}
