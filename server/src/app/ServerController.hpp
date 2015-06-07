// SVC - Simple Voice Communicator 
// kontroler serwera dla połączeń przez NAT
// autor: Filip Gralewski - podstawa sieciowa
// autor: Marcin Frankowski - logika serwera

#ifndef SERVERCONTROLLER_HPP
#define SERVERCONTROLLER_HPP

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
#include "ClientsMap.hpp"
#include <thread>
#include <list>


class ServerController
{
private:

   struct sockaddr_in myAddr;
   struct sockaddr_in clientAddr;
   int TCPSocket;
   int TCPSockSend;
   int UDPSocket;
   ClientsMap clientsMap;
   int connFD;
   std::fstream networkLog;
   std::list<std::thread> threadList;
   void writeLogErrno(std::string message);
   void writeLogErrno(std::string message, char* msg);
   void writeLogErrno(std::string message, const char* msg);
   void writeLog(std::string message);
   void writeLogInt(std::string message, int value);
   void writeLog(std::string message, char* msg);

public:

   ServerController();
   ~ServerController();
   int initTcpServer(int port);
   int acceptTcpConnections(int port);
   int fillUDPConnectionData();
   int recvTCP(int sock, char* msg, int size, const char* function);
   void sendTCP(int sock, const char* msgbuf, const char* function);
   void singleConnectionHandler(std::string login);
   void getUsersList();
   void printUsers();
   void cleanSockets();
};

#endif