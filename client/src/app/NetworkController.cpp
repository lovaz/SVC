
#include "NetworkController.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int Connection::tcpServer(int port)
{
   struct sockaddr_in stSockAddr;
   connFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if(connFD == -1)
   {
      printf("Nie udalo sie utworzyc gniazda TCP.\n");
      return -1;
   }
   else
      printf("Gniazdo TCP zostalo stworzone.\n");

   memset(&stSockAddr, 0, sizeof(stSockAddr));
    
   stSockAddr.sin_family = AF_INET;                      //ipv4, dla ipv6 - AF_INET6
   stSockAddr.sin_port = htons(port);                    //port
   stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);       //adres nasluchiwania
   
   
   if(bind(connFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
   {
      printf("Nie udało się wykonać funkcji bind dla stworzonego gniazda.\n");
      close(connFD);
      return -1;
   }
   else
      printf("Funkcja bind zakończona poprawnie.\n");
    
   if(-1 == listen(connFD, 10))
   {
      perror("Nasłuchiwanie nieudane.\n");
      close(connFD);
      return -1;
   }
   else
      printf("Oczekuję na połączenie...\n");
   
   TCPSocketFD = accept(connFD, NULL, NULL);
   
   if(0 > TCPSocketFD)
   {
      printf("Nie udalo sie zaakceptowac polaczenia.\n");
      return -1;
   }
   else
   {
      isConnected = true;
      printf("Polaczenie zaakceptowane.\n");
   }
    close(connFD);
   return 0;
}

int Connection::tcpClient(int port, char addr[], int addr_length)
{
   struct sockaddr_in stSockAddr;
   int Res;
   TCPSocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

   if(TCPSocketFD == -1)
   {
      printf("Nie udalo sie utworzyc gniazda TCP.\n");
      return -1;
   }
   else
      printf("Gniazdo TCP zostalo stworzone.\n");
    
   memset(&stSockAddr, 0, sizeof(stSockAddr));
    
   stSockAddr.sin_family = AF_INET;
   stSockAddr.sin_port = htons(port);
   Res = inet_pton(AF_INET, addr, &stSockAddr.sin_addr);
    
   if (0 > Res)
   {
      printf("Zla rodzina adresow IP.\n");
      close(TCPSocketFD);
      return -1;
   }
   else if (0 == Res)
   {
      printf("Bledny adres IP.\n");
      close(TCPSocketFD);
      return -1;
   }
   if (-1 == connect(TCPSocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
   {
      printf("Nie udalo sie polaczyc z %s:%d\n", addr, port);
      close(TCPSocketFD);
      return -1;
   }
   else
   {
      isConnected = true;
      printf("Polaczono z %s:%d\n", addr, port);
   }

   return 0;
}

int Connection::shutdownTcpConnection()
{
   if (shutdown(TCPSocketFD, SHUT_RDWR))
   {
      printf("Nie moge zamknac gniazda.\n");
      close(TCPSocketFD);
      return -1;
   }
   
   close(TCPSocketFD);
   printf("Polaczenie TCP zamkniete\n");
   return 0;
}

int Connection::tcpSend()
{
  if(!isConnected)
  {
    printf("Brak polaczenia.\n");
    return -1;
  }
  while(1)
    {
        printf("Wpisz polecenie: ");
        scanf("%s" , msg);
        if(strcmp(msg,"createUdpSockets") == 0)
        {
          createUdpSockets(8001,8002);
        }
        if( send(TCPSocketFD , msg , strlen(msg) , 0) < 0)
        {
            puts("Wysylanie nie powiodlo sie\n");
            return 1;
        }
         
        if( recv(TCPSocketFD , msg , 100 , 0) < 0)
        {
            puts("Odbieranie nie powiodlo sie.\n");
            break;
        }
        else if(strcmp(msg,"exitOK") == 0)
        {
          shutdownTcpConnection();
          break;
        }
         
        printf("server: %s\n",msg);
    }
  return 0;
}

int Connection::tcpRecv()
{
  if(!isConnected)
  {
    printf("Brak polaczenia.\n");
    return -1;
  }
  int read_size;
  while( (read_size = recv(TCPSocketFD , msg , 100 , 0)) > 0 )
    {
        if(strcmp(msg,"myUdpPorts") == 0)
        {
          //saveUdpPorts(msg);
        }
        if(strcmp(msg,"createUdpSockets") == 0)
        {
          createUdpSockets(8003,8004);
        }
        else if(strcmp(msg,"exit") == 0)
        {
          strcpy(msg, "exitOK");
          write(TCPSocketFD , msg , strlen(msg));
          shutdownTcpConnection();
          break;
        }
        write(TCPSocketFD , msg , strlen(msg));
    }
  
  return 0;
}

int Connection::createUdpSockets(int portSend, int portRecv)
{
   struct sockaddr_in stSockAddr;
  udpConnection.myRecvPortFD.socket = socket(AF_INET, SOCK_DGRAM, 0);
   if (udpConnection.myRecvPortFD.socket == -1)
   {
      printf("Nie udalo sie stworzyc gniazda UDP\n");
      return -1;
   }
   else
      printf("Gniazdo UDP zostalo utworzone\n");
  udpConnection.mySendPortFD.socket = socket(AF_INET, SOCK_DGRAM, 0);
   if (udpConnection.mySendPortFD.socket == -1)
   {
      printf("Nie udalo sie stworzyc gniazda UDP\n");
      return -1;
   }
   else
      printf("Gniazdo UDP zostalo utworzone\n");

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddr.sin_port = htons(udpConnection.myRecvPortFD.port);
    udpConnection.myRecvPortFD.port = portRecv;
    if(bind(udpConnection.myRecvPortFD.socket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      printf("Nie udalo sie wykonac funkcji bind\n");
      return -1;
    }
    else
      printf("Funkcja bind zakonczona poprawnie, port: %d.\n", udpConnection.myRecvPortFD.port);
    stSockAddr.sin_port = htons(udpConnection.mySendPortFD.port);
    udpConnection.mySendPortFD.port = portSend;
    if(bind(udpConnection.mySendPortFD.socket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      printf("Nie udalo sie wykonac funkcji bind\n");
      return -1;
    }
    else
      printf("Funkcja bind zakonczona poprawnie, port: %d.\n", udpConnection.mySendPortFD.port);
    return 0;
}

int Connection::udpSend(char* buffer)
{
  
  return 0;
}

int Connection::udpRecv()
{
  
  return 0;
}