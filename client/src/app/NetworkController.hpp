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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "BlockingQueue.hpp"

#define MAX_BUF 1000000


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
   struct sockaddr_in serverAddrSend;
   struct sockaddr_in myAddrRecv;
   char clientIpAddress[15];
};


class Connection
{
private:
   UdpConnection udpConnection;
   int TCPSocketFD; // gniazdo TCP
   int connFD;
   char msg[1000];
   bool isConnected;
   bool isTcpClient;
public:;
   Connection() { isTcpClient = isConnected = false;}
   int tcpServer(int port);
   int tcpClient(int port, char* addr, int addr_length);
   int shutdownTcpConnection();
   void tcpSend();
   void tcpRecv();
   int createUdpSockets(int portRecv);
   int udpSend(char* buffer);
   int udpRecv();
   int initializeSockaddrStruct(int port);
};

#endif
