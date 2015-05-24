#ifndef NETWORKCONTROLLER_HPP
#define NETWORKCONTROLLER_HPP

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fstream>

#include "BlockingQueue.hpp"
#include "Sample.hpp"


class ClientApp;

struct socketPort
{
   int socket;
   int port;
};

struct UdpConnection
{
   int mySendSocketFD;
   socketPort myRecvSocket;
   struct sockaddr_in serverAddrRecv;
   struct sockaddr_in myAddrRecv;
   char clientIpAddress[15];
   char serverIpAddress[15];
};


class NetworkController
{
private:
   ClientApp* clientApp;
   UdpConnection udpConnection;
   int TCPSockSend;
   int TCPSockRecv; 
   int connFD;
   int error;
   char tcpMsg[100];
   bool isConnected;
   bool areUdpSocketsCreated;
   bool isTcpClient;
   bool recvUDPThread;
   bool sendUDPThread;
   bool passivSide;
   bool activSide;
   std::fstream networkLog;

public:
   NetworkController(ClientApp* clientApp);
   ~NetworkController();
   int tcpServer(int port);
   int tcpClient(int port, char* addr, char addrFamily[4]);
   int shutdownTcpConnection();
   int shutdownUdpConnection();
   int udpConnect();
   int endConnection();
   int createUdpSockets(int portRecv);
   int udpSend(BlockingQueue<Sample>& blockingQueue);
   int udpRecv(BlockingQueue<Sample>& blockingQueue);
   int initializeSockaddrStruct(int port);
   void tcpRecv();
   void getMyIp();
   void stopSendUDPThread();
   void stopRecvUDPThread();
   void startSendUDPThread();
   void startRecvUDPThread();
   void reset();
   void sampleFactory(BlockingQueue<Sample>& blockingQueue);
};

#endif