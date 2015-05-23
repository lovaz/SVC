#include "NetworkController.hpp"

int NetworkController::tcpServer(int port)
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
    int readSize = recv(TCPSocketFD , tcpMsg , 100 , 0);
    tcpMsg[readSize] = '\0';
    strcpy(udpConnection.clientIpAddress, tcpMsg);
   return 0;
}

int NetworkController::tcpClient(int port, char addr[], char addrFamily[4])
{
   struct sockaddr_in stSockAddr;
   int Res;
   TCPSocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   strcpy(udpConnection.serverIpAddress,addr);
   isTcpClient = true;
   if(TCPSocketFD == -1)
   {
      printf("Nie udalo sie utworzyc gniazda TCP.\n");
      return -1;
   }
   else
      printf("Gniazdo TCP zostalo stworzone.\n");
    
   memset(&stSockAddr, 0, sizeof(stSockAddr));
   
   if(strcmp(addrFamily, "ipv4") == 0)
    stSockAddr.sin_family = AF_INET;
   else if (strcmp(addrFamily, "ipv6") == 0)
    stSockAddr.sin_family = AF_INET6;
   else
   {
      printf("Zla rodzina adresow IP.\n");
      close(TCPSocketFD);
      return -1;
   }
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

    //ustalenie adresu klienta
    getMyIp();
    strcpy(tcpMsg, udpConnection.clientIpAddress);
    send(TCPSocketFD , tcpMsg , strlen(tcpMsg) , 0);
   return 0;
}

int NetworkController::shutdownTcpConnection()
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

int NetworkController::udpConnect()
{
  if(!isConnected)
  {
    printf("Brak połączenia TCP.\n");
    return -1;
  }
  if(areUdpSocketsCreated)
  {
    printf("Połączenie UDP jest juz stworzone.\n");
    return -1;
  }
  int readSize;
  strcpy(tcpMsg, "connect");
  if( send(TCPSocketFD , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
        printf("Tworzenie połączenia UDP nie powiodło się. Nie można wysłać żądania do serwera.\n");
        return -1;
    }
  if( (readSize = recv(TCPSocketFD , tcpMsg , 100 , 0)) < 0)
    {
        printf("Tworzenie połączenia UDP nie powiodło się. Nie otrzymano odpowiedzi z serwera.\n");
        return -1;
    }
  tcpMsg[readSize] = '\0';
  if(createUdpSockets(50001) != 0)
    {
        printf("Tworzenie połączenia UDP nie powiodło się. Nie udało się stworzyć gniazda UDP.\n");
        return -1;
    }
  initializeSockaddrStruct(50002);
  areUdpSocketsCreated = true;
  printf("Połączenie UDP zostało poprawnie stworzone\n");
  printf("Wiadomość z serwera: %s\n", tcpMsg);
  return 0;
}

int NetworkController::endConnection()
{
  if(!isConnected)
    exit(0);
  int readSize;
  strcpy(tcpMsg, "exit");
  if( send(TCPSocketFD , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
        printf("Kończenie połączenia nie powiodło się. Nie można wysłać żądania do serwera.\n");
        return -1;
    }
  if( (readSize = recv(TCPSocketFD , tcpMsg , 100 , 0)) < 0)
    {
        printf("Kończenie połączenia nie powiodło się. Nie otrzymano odpowiedzi z serwera.\n");
        return -1;
    }
  tcpMsg[readSize] = '\0';
  if(strcmp(tcpMsg,"exitOK") == 0)
    {
      shutdownTcpConnection();
      shutdownUdpConnection();
    }
  else
  {
    printf("Kończenie połączenia nie powiodło się. Nie otrzymano potwierdzenia z serwera.\n");
    return -1;
  }
  exit(0);
}

void NetworkController::tcpRecv()
{
  if(!isConnected)
  {
    printf("Brak polaczenia.\n");
    return;
  }
  int readSize;
  while( (readSize = recv(TCPSocketFD , tcpMsg , 100 , 0)) > 0 )
    {
      tcpMsg[readSize] = '\0';
      if(strcmp(tcpMsg,"connect") == 0)
      {
        createUdpSockets(50002);
        initializeSockaddrStruct(50001);
        sprintf(tcpMsg,"UDP socket created. Bind with port 50002");
      }
      else if(strcmp(tcpMsg,"exit") == 0)
      {
        endUdpCommunication = true;
        strcpy(tcpMsg, "exitOK");
        send(TCPSocketFD , tcpMsg , strlen(tcpMsg), 0);
        shutdownTcpConnection();
        shutdownUdpConnection();
        exit(0);
      }
      send(TCPSocketFD , tcpMsg , strlen(tcpMsg), 0);
    }
}

int NetworkController::createUdpSockets(int portRecv)
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

int NetworkController::initializeSockaddrStruct(int port)
{
  udpConnection.serverAddrRecv.sin_family = AF_INET;
  udpConnection.serverAddrRecv.sin_port = htons(port);
  if(isTcpClient)
    inet_pton(AF_INET, udpConnection.serverIpAddress , &udpConnection.serverAddrRecv.sin_addr);
  else
    inet_pton(AF_INET, udpConnection.clientIpAddress , &udpConnection.serverAddrRecv.sin_addr);
  return 0;
}

int NetworkController::udpSend(BlockingQueue<Sample>& blockingQueue)
{
  while(!endUdpCommunication)
  {
    char buffer[MAX_BUF];
    strcpy(buffer,blockingQueue.pop().getSample());
    sendto(udpConnection.mySendSocketFD, buffer, strlen(buffer)+1, 0, 
        (struct sockaddr*)&udpConnection.serverAddrRecv, sizeof(udpConnection.serverAddrRecv));
  }
  return 0;
}

int NetworkController::udpRecv(BlockingQueue<Sample>& blockingQueue)
{
  char buf[MAX_BUF];
  while(!endUdpCommunication)
  {
    unsigned int addrlen = sizeof(udpConnection.myAddrRecv);
    int i = recvfrom(udpConnection.myRecvSocket.socket, buf, MAX_BUF, 0, 
        (struct sockaddr*)&udpConnection.myAddrRecv, &addrlen);
    if(i > 0)
    {
      Sample sample;
      sample.setSample(buf);
      blockingQueue.push(sample);
      printf("%s\n",sample.getSample());
      fflush(stdout);
    }
  }
  return 0;
}

int NetworkController::shutdownUdpConnection()
{
   close(udpConnection.mySendSocketFD);
   close(udpConnection.myRecvSocket.socket);

   printf("Polaczenie UDP zamkniete\n");
   return 0;
}

void NetworkController::getMyIp()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);
    int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*) &name, &namelen);
    assert(err != -1);
    char buffer[15];
    strcpy(udpConnection.clientIpAddress, inet_ntop(AF_INET, &name.sin_addr, buffer, 15));
    close(sock);
}


//funkcja pomocnicza
void NetworkController::sampleFactory(BlockingQueue<Sample>& blockingQueue)
{
  while(1)
  {
    fflush(stdout);
    Sample sample;
    char* message = (char*)malloc(100*sizeof(char));
    strcpy(message,"testMessage");
    sample.setSample(message);
    if(message!=NULL)
      free(message);
    blockingQueue.push(sample);
    usleep(200000);
  }
}
