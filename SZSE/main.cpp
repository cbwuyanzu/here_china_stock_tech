#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "admin.h"

#define SERVER_IP "172.24.2.50"
#define PORT 18016
#define BUFFER_SIZE 1024
#define TOTAL_STEP  10

int main(){
  // 创建socket
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    std::cerr << "Socket creation failed" << std::endl;
    return 1;
  }
   // 设置服务器地址
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  
  if(connect(clientSocket,(sockaddr*)&serverAddr,sizeof(serverAddr)) == -1){
    std::cerr << "Connection failed" << std::endl;
    close(clientSocket);
    return 1;
  }
  std::cout << "step:1/"<< TOTAL_STEP << "\tConnect to server" << std::endl;
  
  if(SendLogon(clientSocket) != 0){
    std::cerr << "Logon failed" << std::endl;
    return 2;
  }
  std::cout << "step:2/"<< TOTAL_STEP << "\tLogon Send" << std::endl;

  if(RecvLogon(clientSocket) != 0){
    std::cerr << "Logon failed" << std::endl;
    return 3;
  }
  std::cout << "step:3/"<< TOTAL_STEP << "\tLogon Done" << std::endl;

  std::cout << "step:4/"<< TOTAL_STEP << "\tStart Receive MarketData" << std::endl;
  int bexit = 0;
  while(bexit == 0){
    bexit = RecvMsg(clientSocket);
  } 
  std::cout << "step:last/"<< TOTAL_STEP << "\tTo Exit bexit=" << bexit << std::endl;
  close(clientSocket);
  return 0;
}
