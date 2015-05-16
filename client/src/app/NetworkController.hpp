#ifndef NETWORKCONTROLLER_HPP
#define NETWORKCONTROLLER_HPP

#include "BlockingQueue.hpp"

struct socketPort
{
   int socket;
   int port;
};

struct UdpConnection
{
   socketPort myRecvPortFD;
   socketPort mySendPortFD;
   socketPort clientRecvPortFD;
   socketPort clientSendPortFD;
   char clientIpAddress[15];
};


class Connection
{
private:
   UdpConnection udpConnection;
   int TCPSocketFD; // gniazdo TCP
   int connFD;
   char msg[100];
   bool isConnected;
public:;
   Connection() { isConnected = false;}
   int tcpServer(int port);
   int tcpClient(int port, char* addr, int addr_length);
   int shutdownTcpConnection();
   int tcpSend();
   int tcpRecv();
   int createUdpSockets(int portSend, int portRecv);
   int udpSend(char* buffer);
   int udpRecv();
};

#endif
