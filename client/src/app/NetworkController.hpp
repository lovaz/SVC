// SVC - Simple Voice Communicator 
// Kontroler połączenia sieciowego
// autor: Filip Gralewski - podstawa sieci
// autor: Beata Dobrzyńska - rozwinięcie funkcjonalności klienta


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
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <fstream>
#include <thread>
#include <list>


#include "BlockingQueue.hpp"
#include "Sample.hpp"


class ClientApp;

struct socketPort
{
   int socket;
   int port;
};


class NetworkController
{
private:
   ClientApp* clientApp;
   socketPort bindedUDPSocket;
   int holePunchSocket;
   struct sockaddr_in serverAddr;
   struct sockaddr_in holePunchAddr;
   char myIPAddress[15];
   char serverIPAddress[15];
   int TCPSockSend;
   int TCPSockRecv; 
   int connFD;
   bool passivSide;
   bool activSide;
   bool isConnected;
   bool recvUDPThread;
   bool sendUDPThread;
   std::fstream networkLog;
   std::list<std::thread> threadList;
   void writeLogErrno(std::string message);
   void writeLogErrno(std::string message, char* msg);
   void writeLogErrno(std::string message, const char* msg);
   void writeLog(std::string message);
   void writeLogInt(std::string message, int value);
   void writeLog(std::string message, char* msg);
   int connectTimeout(int sec);
   void setServerAddr(int port);
   int recvHolePunchData(int sock);
   void setHolePunchAddr(int host, short port);
   void sendKillPackets(int mode);

public:

   NetworkController(ClientApp* clientApp);
   ~NetworkController();
   int createUdpSocket(int portRecv);
   int initTcpServer(int port);
   int acceptTcpConnections(int port);
   int connectHost(int port, char* addr, const char* addrFamily);
   int connectServer(int port, char* addr, char* login);
   int callPeerRequest(std::string login);
   int shutdownTcpConnection();
   void closeUDPSockets();
   int startUDPNetTransmission();
   int startUDPNatTransmission();
   void tcpRecvFromHost();
   void tcpRecvFromServer();
   int udpSend(BlockingQueue<Sample>& blockingQueue, int mode);
   int udpRecv(BlockingQueue<Sample>& blockingQueue, int mode);
   void sendTCP(int sock, const char* msgbuf, const char* function);
   int recvTCP(int sock, char* msg, int size, const char* function);
   void logoutFromServer();
   void getMyIp();
   void getHolePunchData();
   void getUsersList();
   void endNetConnection();
   void endNatConnection();
   void stopSendUDPThread();
   void stopRecvUDPThread();
   void startSendUDPThread();
   void startRecvUDPThread();
};

#endif