#include "NetworkController.hpp"

#define RECV_ADDR "192.168.1.2"

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
   strcpy(udpConnection.clientIpAddress,addr);
   isTcpClient = true;
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

void Connection::tcpSend()
{
  if(!isConnected)
  {
    printf("Brak polaczenia.\n");
    return;
  }
  int read_size;
  while(1)
    {
        printf("Wpisz polecenie: ");
          scanf("%s" , msg);

        if(strcmp(msg,"connect") == 0)
        {
          createUdpSockets(50001);
          initializeSockaddrStruct(50002);
        }

        if( send(TCPSocketFD , msg , strlen(msg) , 0) < 0)
        {
            puts("Wysylanie nie powiodlo sie\n");
            return;
        }
         
        if( (read_size = recv(TCPSocketFD , msg , 100 , 0)) < 0)
        {
            puts("Odbieranie nie powiodlo sie.\n");
            break;
        }
        else if(strcmp(msg,"exitOK") == 0)
        {
          shutdownTcpConnection();
          exit(0);
        }
        msg[read_size] = '\0';
        printf("server: %s\n",msg);
    }
}

void Connection::tcpRecv()
{
  if(!isConnected)
  {
    printf("Brak polaczenia.\n");
    return;
  }
  int read_size;
  while( (read_size = recv(TCPSocketFD , msg , 100 , 0)) > 0 )
    {
      msg[read_size] = '\0';
      if(strcmp(msg,"myUdpPorts") == 0)
      {
        //saveUdpPorts(msg);
      }
      if(strcmp(msg,"connect") == 0)
      {
        createUdpSockets(50002);
        initializeSockaddrStruct(50001);
        sprintf(msg,"socketsOK::50002");
      }
      else if(strcmp(msg,"exit") == 0)
      {
        strcpy(msg, "exitOK");
        send(TCPSocketFD , msg , strlen(msg), 0);
        shutdownTcpConnection();
        break;
      }
      send(TCPSocketFD , msg , strlen(msg), 0);
    }
    exit(0);
}

int Connection::createUdpSockets(int portRecv)
{

  udpConnection.myRecvSocket.port = portRecv;

  struct sockaddr_in stSockAddr;
  udpConnection.mySendSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
   if (udpConnection.mySendSocketFD == -1)
   {
      printf("Nie udalo sie stworzyc gniazda UDP\n");
      return -1;
   }
   else
      printf("Gniazdo UDP zostalo utworzone\n");
  udpConnection.myRecvSocket.socket = socket(AF_INET, SOCK_DGRAM, 0);
   if (udpConnection.myRecvSocket.socket == -1)
   {
      printf("Nie udalo sie stworzyc gniazda UDP\n");
      return -1;
   }
   else
      printf("Gniazdo UDP zostalo utworzone\n");

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddr.sin_port = htons(udpConnection.myRecvSocket.port);
    if(bind(udpConnection.myRecvSocket.socket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      printf("Nie udalo sie wykonac funkcji bind\n");
      return -1;
    }
    else
      printf("Funkcja bind zakonczona poprawnie, port: %d.\n", udpConnection.myRecvSocket.port);

    fflush(stdout);
    return 0;
}

int Connection::initializeSockaddrStruct(int port)
{
  udpConnection.serverAddrRecv.sin_family = AF_INET;
  udpConnection.serverAddrRecv.sin_port = htons(port);
  if(isTcpClient)
    inet_pton(AF_INET, udpConnection.clientIpAddress , &udpConnection.serverAddrRecv.sin_addr);
  else
    inet_pton(AF_INET, RECV_ADDR , &udpConnection.serverAddrRecv.sin_addr);
  return 0;
}

int Connection::udpSend(char* buffer)
{
  while(1)
  {
    sendto(udpConnection.mySendSocketFD, buffer, strlen(buffer)+1, 0, 
        (struct sockaddr*)&udpConnection.serverAddrRecv, sizeof(udpConnection.serverAddrRecv));
    fflush(stdout);
    sleep(1);
  }
  return 0;
}

int Connection::udpRecv()
{
  while(1)
  {
    char buf[MAX_BUF];
    unsigned int addrlen = sizeof(udpConnection.myAddrRecv);
    int i = recvfrom(udpConnection.myRecvSocket.socket, buf, MAX_BUF, 0, 
        (struct sockaddr*)&udpConnection.myAddrRecv, &addrlen);
    if(i > 0)
      printf("%s\n", buf);
    fflush(stdout);
    sleep(1);
  }
  return 0;
}