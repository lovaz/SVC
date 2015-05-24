#include "NetworkController.hpp"
#include "ClientApp.hpp"


NetworkController::NetworkController(ClientApp* clientApp) : clientApp(clientApp)
{ 
    passivSide = false;
    activSide = false;
    isTcpClient = false;
    isConnected = false;
    areUdpSocketsCreated = false;
    recvUDPThread = false;
    sendUDPThread = false;
    networkLog.open("network.log", std::ios::out | std::ios::trunc);
    if(networkLog.good() != true)
    {
      std::cout << "Nie udało się otworzyć pliku network.log"<<std::endl;
    }
}

NetworkController::~NetworkController()
{
    if(networkLog.good() == true)
    {
      networkLog.close();
    }
}



int NetworkController::tcpServer(int port)
{
    struct sockaddr_in stSockAddr;
    connFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(connFD == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpServer::Cannot create socket, errno: "<<errno<<std::endl;
      }
      return -1;
    }

    memset(&stSockAddr, 0, sizeof(stSockAddr));
    
    stSockAddr.sin_family = AF_INET;                      //ipv4, dla ipv6 - AF_INET6
    stSockAddr.sin_port = htons(port);                    //port
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);       //adres nasluchiwania
   
    if(bind(connFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpServer::Cannot bind, errno: "<<errno<<std::endl;
      }
      close(connFD);
      return -1;
    }
    
    if(-1 == listen(connFD, 10))
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpServer::Cannot listen, errno: "<<errno<<std::endl;
      }
      close(connFD);
      return -1;
    }
    else
    {
      printf("Oczekuję na połączenie...\n");
    }
   
    TCPSockRecv = accept(connFD, NULL, NULL);
   
    if(0 > TCPSockRecv)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpServer::Cannot accept, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    else
    {
      isConnected = true;
      printf("Polaczenie zaakceptowane.\n");
    }
    close(connFD);
    int readSize = recv(TCPSockRecv , tcpMsg , 100 , 0);
    tcpMsg[readSize] = '\0';
    strcpy(udpConnection.clientIpAddress, tcpMsg);
    if(activSide == false)
    {
      passivSide = true;
      tcpClient(port, udpConnection.clientIpAddress, "ipv4");
    }
    if(activSide == true)
    {
      if(strcmp(tcpMsg, udpConnection.serverIpAddress) != 0)
      {
        close(TCPSockRecv);
        return -1;
      }
    }
    if(networkLog.good() == true)
    {
      networkLog<<"tcpServer::OK"<<std::endl;
    }
    return 0;
}

int NetworkController::tcpClient(int port, char addr[], char addrFamily[4])
{
    if(passivSide == false)
    activSide = true;
    struct sockaddr_in stSockAddr;
    int Res;
    TCPSockSend = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    strcpy(udpConnection.serverIpAddress,addr);
    isTcpClient = true;
    if(TCPSockSend == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpClient::Cannot create socket, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    
    memset(&stSockAddr, 0, sizeof(stSockAddr));
   
    if(strcmp(addrFamily, "ipv4") == 0)
    {
      stSockAddr.sin_family = AF_INET;
    }
    else if (strcmp(addrFamily, "ipv6") == 0)
    {
      stSockAddr.sin_family = AF_INET6;
    }
    else
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpClient::Invalid address family"<<std::endl;
      }
      close(TCPSockSend);
      return -1;
    }
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, addr, &stSockAddr.sin_addr);
    
    if (0 > Res)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpClient::Invalid address family"<<std::endl;
      }
      close(TCPSockSend);
      return -1;
    }
    else if (0 == Res)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpClient::Invalid IP address"<<std::endl;
      }
      close(TCPSockSend);
      return -1;
    }
    if (-1 == connect(TCPSockSend, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      if(networkLog.good() == true)
      {
        networkLog<<"tcpClient::Cannot connect, errno: "<<errno<<std::endl;
      }
      close(TCPSockSend);
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
    send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0);
    if(networkLog.good() == true)
    {
      networkLog<<"OK"<<std::endl;
    }
    return 0;
}

int NetworkController::shutdownTcpConnection()
{
    int ret1 = shutdown(TCPSockSend, SHUT_RDWR);
    int ret2 = shutdown(TCPSockRecv, SHUT_RDWR);
    if (ret1 || ret2)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot close sockets, errno: "<<errno<<std::endl;
      }
      close(TCPSockSend);
      close(TCPSockRecv);
      return -1;
    }
    close(TCPSockRecv);
    close(TCPSockSend);
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
    if(send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot create UDP connection. Cannot send request to server, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    if((readSize = recv(TCPSockSend , tcpMsg , 100 , 0)) < 0)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot create UDP connection. Cannot receieve from server, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    tcpMsg[readSize] = '\0';
    if(createUdpSockets(50001) != 0)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot create UDP connection. Cannot create socket, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    initializeSockaddrStruct(50002);
    areUdpSocketsCreated = true;
    return 0;
}

int NetworkController::endConnection()
{
    if(!isConnected)
    {
      return 0;
    }
    int readSize;
    strcpy(tcpMsg, "exit");
    if(send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot end connection. Cannot send request to server, errno: "<<errno<<std::endl;
      }
      return -1;
    }
    if((readSize = recv(TCPSockSend , tcpMsg , 100 , 0)) < 0)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot end connection. Cannot receive from server, errno: "<<errno<<std::endl;
      }
    }
    tcpMsg[readSize] = '\0';
    if(strcmp(tcpMsg,"exitOK") == 0)
    {
      shutdownTcpConnection();
      shutdownUdpConnection();
    }
    else
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot end connection. Cannot send request to server, errno: "<<errno<<std::endl;
      }
    }
    return 0;
}

void NetworkController::tcpRecv()
{
    if(!isConnected)
    {
      printf("Brak polaczenia!\n");
      return;
    }
    int readSize;
    while((readSize = recv(TCPSockRecv , tcpMsg , 100 , 0)) > 0)
    {
      tcpMsg[readSize] = '\0';
      if(strcmp(tcpMsg,"connect") == 0)
      {
        createUdpSockets(50002);
        initializeSockaddrStruct(50001);
        sprintf(tcpMsg,"UDP socket created. Bind with port 50002");
        clientApp->startCallViaNet();

      }
      else if(strcmp(tcpMsg,"exit") == 0)
      {
        strcpy(tcpMsg, "exitOK");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        shutdownTcpConnection();
        shutdownUdpConnection();
        clientApp->endCallViaNet();
      }
      send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
    }
}

int NetworkController::createUdpSockets(int portRecv)
{
    udpConnection.myRecvSocket.port = portRecv;

    struct sockaddr_in stSockAddr;
    udpConnection.mySendSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpConnection.mySendSocketFD == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot create UDP socket, errno: "<<errno<<std::endl;
      }
      return -1;
    }

    udpConnection.myRecvSocket.socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpConnection.myRecvSocket.socket == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot create UDP socket, errno: "<<errno<<std::endl;
      }
      return -1;
    }

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddr.sin_port = htons(udpConnection.myRecvSocket.port);
    if(bind(udpConnection.myRecvSocket.socket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      if(networkLog.good() == true)
      {
        networkLog<<"Cannot bind, errno: "<<errno<<std::endl;
      }
      return -1;
    }

    fflush(stdout);
    return 0;
}

int NetworkController::initializeSockaddrStruct(int port)
{
    udpConnection.serverAddrRecv.sin_family = AF_INET;
    udpConnection.serverAddrRecv.sin_port = htons(port);
    if(isTcpClient)
    {
      inet_pton(AF_INET, udpConnection.serverIpAddress , &udpConnection.serverAddrRecv.sin_addr);
    }
    else
    {
      inet_pton(AF_INET, udpConnection.clientIpAddress , &udpConnection.serverAddrRecv.sin_addr);
    }
    return 0;
}

int NetworkController::udpSend(BlockingQueue<Sample>& blockingQueue)
{
    float buffer[FRAMES_PER_BUFFER];
    while(sendUDPThread)
    {
      memcpy(buffer,blockingQueue.pop().getSample(), sizeof(buffer));
      if(sendto(udpConnection.mySendSocketFD, buffer,FRAMES_PER_BUFFER * sizeof(float), 0, 
        (struct sockaddr*)&udpConnection.serverAddrRecv, sizeof(udpConnection.serverAddrRecv)) < 0)
      {
        if(networkLog.good() == true)
        {
          networkLog<<"udpSend error, errno: "<<errno<<std::endl;
        }
      } 
    }
    return 0;
}

int NetworkController::udpRecv(BlockingQueue<Sample>& blockingQueue)
{
    int i = 0;
    Sample sample;
    float buf[FRAMES_PER_BUFFER];
    while(recvUDPThread)
    {
      unsigned int addrlen = sizeof(udpConnection.myAddrRecv);
      i = recvfrom(udpConnection.myRecvSocket.socket, buf, FRAMES_PER_BUFFER * sizeof(float), 0, 
        (struct sockaddr*)&udpConnection.myAddrRecv, &addrlen);
      if(i > 0)
      {
        sample.setSample(buf);
        blockingQueue.push(sample);
        fflush(stdout);
      }
      if(i < 0)
      {
        if(networkLog.good() == true)
        {
          networkLog<<"udpRecv error, errno: "<<errno<<std::endl;
        }
      }
    }
    return 0;
}

int NetworkController::shutdownUdpConnection()
{
    close(udpConnection.mySendSocketFD);
    close(udpConnection.myRecvSocket.socket);
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


void NetworkController::stopSendUDPThread()
{
    sendUDPThread = false;
}
   

void NetworkController::stopRecvUDPThread()
{
    recvUDPThread = false;
}

void NetworkController::startSendUDPThread()
{
    sendUDPThread = true;
}
   

void NetworkController::startRecvUDPThread()
{
    recvUDPThread = true;
}

void NetworkController::reset()
{
    passivSide = false;
    activSide = false;
    isTcpClient = false;
    isConnected = false;
    areUdpSocketsCreated = false;
    recvUDPThread = false;
    sendUDPThread = false;
    shutdownTcpConnection();
    shutdownUdpConnection();
}


//funkcja pomocnicza
void NetworkController::sampleFactory(BlockingQueue<Sample>& blockingQueue)
{
  int i = 0;
  while(1)
  {
    fflush(stdout);
    Sample sample;
    float* message = (float*)malloc(FRAMES_PER_BUFFER*sizeof(float));
    {
      message[i] = 0.00001 * i;
    }
    sample.setSample(message);
    if(message!=NULL)
      free(message);
    blockingQueue.push(sample);
    sleep(2);
  }
}


