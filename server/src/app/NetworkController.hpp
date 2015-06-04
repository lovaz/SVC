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
#include "ThreadSafeMap.hpp"
#include "Sample.hpp"
#include <thread>
#include <list>






class NetworkController
{
private:

   struct sockaddr_in myAddr;
   struct sockaddr_in clientAddr;
   int TCPSocket;
   int TCPSockSend;
   int UDPSocket;
   char buf[512];
   ThreadSafeMap clientsMap;
   int connFD;
   int error;
   char tcpMsg[100];
   bool isConnected;
   bool isServerInitialized;
   std::fstream networkLog;
   std::list<std::thread> threadList;
   void writeLogErrno(std::string message);
   void writeLogErrno(std::string message, char* msg);
   void writeLogErrno(std::string message, const char* msg);
   void writeLog(std::string message);
   void writeLogInt(std::string message, int value);
   void writeLog(std::string message, char* msg);

public:

   NetworkController();
   ~NetworkController();
   int initTcpServer(int port);
   int acceptTcpConnections(int port);
   void singleConnectionHandler(std::string login);
   int fillUDPConnectionData();
   void sendTCP(int sock, const char* msgbuf, const char* function);
   int recvTCP(int sock, char* msg, int size, const char* function);
   void getUsersList();
   void printUsers()
   {
      clientsMap.printMap();
   }
};

#endif