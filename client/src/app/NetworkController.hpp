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

#include "BlockingQueue.hpp"

#define MAX_BUF 1000000

//Struktura skopiowana tymczasowo z pliku AudioController.hpp
class Sample
{
private:
    char sample[1000];
public:
    void setSample(char* data)
    {
        memcpy(sample, data, 1000);
    }
    char* getSample()
    {
        return sample;
    }
};


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
   UdpConnection udpConnection;
   int TCPSocketFD; // gniazdo TCP
   int connFD;
   char tcpMsg[100];
   bool isConnected;
   bool areUdpSocketsCreated;
   bool isTcpClient;
   bool endUdpCommunication;
public:;
   NetworkController() { endUdpCommunication = isTcpClient = isConnected = areUdpSocketsCreated = false;}
   int tcpServer(int port);
   int tcpClient(int port, char* addr, char addrFamily[4]);
   int shutdownTcpConnection();
   int shutdownUdpConnection();
   int udpConnect();
   int endConnection();
   void tcpRecv();
   int createUdpSockets(int portRecv);
   int udpSend(BlockingQueue<Sample>& blockingQueue);
   int udpRecv(BlockingQueue<Sample>& blockingQueue);
   int initializeSockaddrStruct(int port);
   void getMyIp();
   //funkcja pomocnicza
   void sampleFactory(BlockingQueue<Sample>& blockingQueue);
};

#endif
