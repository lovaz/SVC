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
   int error;
   bool isConnected;
   bool areUdpSocketsCreated;
   bool isTcpClient;
   bool recvUDPThread;
   bool sendUDPThread;
   bool recvTCPThread;
   bool passivSide;
   bool activSide;
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

   NetworkController(ClientApp* clientApp);
   ~NetworkController();
   int initTcpServer(int port);
   int acceptTcpConnections(int port);
   int tcpConnect(int port, char* addr, const char* addrFamily);
   int connectServer(int port, char* addr, char* login);
   int shutdownTcpConnection();
   void sendKillPackets(int mode);
   void closeUDPSockets();
   int startUDPNetTransmission();
   int startUDPNatTransmission();
   void endNetConnection();
   void endNatConnection();
   int createUdpSocket(int portRecv);
   int udpSend(BlockingQueue<Sample>& blockingQueue, int mode);
   int udpRecv(BlockingQueue<Sample>& blockingQueue, int mode);
   int udpSend2();
   int udpRecv2();
   void setServerAddr(int port);
   void setHolePunchAddr(int host, short port);
   int recvHolePunchData(int sock);
   void tcpRecv();
   void tcpRecvFromServer();
   void getMyIp();
   void stopSendUDPThread();
   void stopRecvUDPThread();
   void stopRecvTCPThread();
   void startSendUDPThread();
   void startRecvUDPThread();
   void startRecvTCPThread();
   void getHolePunchData();
   void sampleFactory(BlockingQueue<Sample>& blockingQueue);
   int callPeerRequest(std::string login);
   void sendTCP(int sock, const char* msgbuf, const char* function);
   int recvTCP(int sock, char* msg, int size, const char* function);
   void logoutFromServer();
   void getUsersList();
};

#endif