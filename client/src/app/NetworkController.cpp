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
    recvTCPThread = false;
    isServerInitialized = false;
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


int NetworkController::initTcpServer(int port)
{
    if(isServerInitialized == false)
    {
      struct sockaddr_in stSockAddr;
      connFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

      if(connFD == -1)
      {
        writeLogErrno("initTcpServer::Cannot create socket");
        return -1;
      }

      memset(&stSockAddr, 0, sizeof(stSockAddr));
    
      stSockAddr.sin_family = AF_INET;                      //ipv4, dla ipv6 - AF_INET6
      stSockAddr.sin_port = htons(port);                    //port
      stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);       //adres nasluchiwania
   
      if(bind(connFD,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
      {
        writeLogErrno("initTcpServer::Cannot bind");
        close(connFD);
        return -1;
      }
      isServerInitialized = true;
    }

    if(-1 == listen(connFD, 10))
    {
      writeLogErrno("initTcpServer::Cannot listen");
      close(connFD);
      return -1;
    }
    writeLog("initTcpServer::OK");
    return 0;
}



int NetworkController::acceptTcpConnections(int port)
{
  while(1)
  {
    TCPSockRecv = accept(connFD, NULL, NULL);
   
    if(TCPSockRecv < 0)
    {
      writeLogErrno("tcpServer::Cannot accept");
      return -1;
    }
    else
    {
      isConnected = true;
      writeLog("tcpServer::Connection accepted");
    }

    int readSize = recv(TCPSockRecv , tcpMsg , 100 , 0);
    writeLog("tcp recv: ", tcpMsg);
    std::string ip(tcpMsg, readSize);
    if(activSide == false)
    {
      passivSide = true;

      if(clientApp->acceptCall(ip) == 0)
      {        
        std::cout<<"Zaakceptowano"<<std::endl;
        strcpy(udpConnection.clientIpAddress, tcpMsg);
        strcpy(tcpMsg, "CALL_ACCEPTED");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        writeLog("tcp send: ", tcpMsg);
        writeLog("acceptTcpConnections::OK_connected");
        clientApp->commandThread();

        if(tcpConnect(port, &ip[0u], "ipv4") == 0)
        {
          startRecvTCPThread();
          tcpRecv();
        }
        else
        {
          std::cout<<"Błąd połączenia"<<std::endl;
          shutdownTcpConnection();
        }
      }
      else
      {
        std::cout<<"Odrzucono"<<std::endl;
        strcpy(tcpMsg,"CALL_REFUSED");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        writeLog("tcp send: ", tcpMsg);
        shutdownTcpConnection();
        writeLog("acceptTcpConnections::OK_refused");
        clientApp->commandThread();
        activSide = false;
        passivSide = false;
      }
      memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    }
    else
    {  
      char* IP = &ip[0u];
      if(strcmp(IP, udpConnection.serverIpAddress) != 0)
      {
        shutdownTcpConnection();
        writeLog("Unauthorized connection");
        return -1;
      }
      else
      {
        strcpy(tcpMsg,"RETURN_ACCEPTED");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        writeLog("tcp send: ", tcpMsg);
        startRecvTCPThread();
        tcpRecv();
      }
    }
  }
  close(connFD);
  return 0;
}



int NetworkController::tcpConnect(int port, char* addr, const char* addrFamily)
{
    struct sockaddr_in stSockAddr;
    int Res;
    TCPSockSend = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    strcpy(udpConnection.serverIpAddress,addr);
    isTcpClient = true;
    if(TCPSockSend == -1)
    {
      writeLogErrno("initTcpClient::Cannot create socket");
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
      writeLog("initTcpClient::Invalid address family");
      close(TCPSockSend);
      return -2;
    }
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, addr, &stSockAddr.sin_addr);
    
    if (0 > Res)
    {
      writeLog("initTcpClient::Invalid address family");
      close(TCPSockSend);
      return -2;
    }
    else if (0 == Res)
    {
      writeLog("initTcpClient::Invalid IP address");
      close(TCPSockSend);
      return -2;
    }
    writeLog("initTcpClient::OK");

    if (-1 == connect(TCPSockSend, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      writeLogErrno("tcpConnect::Cannot connect");
      close(TCPSockSend);
      return -1;
    }
    else
    {
      isConnected = true;
      writeLog("Connected");
    }

    getMyIp();
    strcpy(tcpMsg, udpConnection.clientIpAddress);
    send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0);
    writeLog("tcp send: ", tcpMsg);

    memset(&tcpMsg[0], 0, sizeof(tcpMsg));
  
    int readSize = recv(TCPSockSend , tcpMsg , 100 , 0);
    writeLog("tcp recv: ", tcpMsg);
    tcpMsg[readSize] = '\0';

    if(passivSide == false)
    {
      if(strcmp(tcpMsg, "CALL_REFUSED") == 0)
      {
        writeLog("tcpConnect::OK_refused");
        close(TCPSockSend);
        activSide = false;
        passivSide = false;
        return 1;
      }
      else if(strcmp(tcpMsg, "CALL_ACCEPTED") == 0)
      {
        writeLog("tcpConnect::OK_accepted");
        activSide = true;
        passivSide = false;
        return 0;
      }
      else
      {
        writeLog("Uknown server command");
        close(TCPSockSend);
        activSide = false;
        passivSide = false;
        return -1;
      }
    }

    if(passivSide == true)
    {
      if(strcmp(tcpMsg, "RETURN_ACCEPTED") == 0)
      {
        writeLog("tcpConnect::OK_return_connection");
        return 0;
      }
      else
      {
        writeLogErrno("tcpConnect::return_connection");
        return -1;
      }
    }
    return -1;
}

int NetworkController::shutdownTcpConnection()
{
    int ret1 = -1;
    int ret2 = -1;
    writeLog("Sockets before shutdown:");
    writeLog("TCPSockRecv: ", TCPSockRecv);
    writeLog("TCPSockSend: ", TCPSockSend);
    if(TCPSockSend > 0)
    {
      ret1 = shutdown(TCPSockSend, SHUT_RDWR);
      writeLog("shutdown TCPSockSend returned: ", ret1);
      TCPSockSend = 0;
    }

    if(TCPSockRecv > 0)
    {
      ret2 = shutdown(TCPSockRecv, SHUT_RDWR);
      writeLog("shutdown TCPSockRecv returned: ", ret2);
      TCPSockRecv = 0;
    }

    if( ret1 == -1 )
    {
      writeLogErrno("TCPSockSend failed");
      if(TCPSockSend > 0)
      {
        close(TCPSockSend);
        TCPSockSend = 0;
      }
      return -1;
    }

    if( ret2 == -1 )
    {
      writeLogErrno("TCPSockRecv failed");
      if (TCPSockRecv > 0)
      {
        close(TCPSockRecv);
        TCPSockRecv = 0;
      }
      return -1;
    }

    if(TCPSockSend > 0)
    {
      close(TCPSockSend);
      TCPSockSend = 0;
    }
    if (TCPSockRecv > 0)
    {
      close(TCPSockRecv);
      TCPSockRecv = 0;
    }
    writeLog("Shutdown done!");
    return 0;
}



int NetworkController::udpConnect()
{
    if(!isConnected)
    {
      std::cout<<"Brak połączenia TCP.\n";
      return -1;
    }
    int readSize;
    memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    strcpy(tcpMsg, "udp_start");
    if(send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
      writeLogErrno("Cannot create UDP connection. Cannot send request to server");
      return -1;
    }
    writeLog("tcp send: ", tcpMsg);
    memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    if((readSize = recv(TCPSockSend , tcpMsg , 100 , 0)) < 0)
    {
      writeLogErrno("Cannot create UDP connection. Cannot receieve from server");
      return -1;
    }
    tcpMsg[readSize] = '\0';
    writeLog("tcp recv: ", tcpMsg);

    if(strcmp(tcpMsg, "UDP_OK") != 0)
    {
      writeLog("UDP error, server side!");
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
    memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    strcpy(tcpMsg, "exit");
    activSide = false;
    passivSide = false;
    if(send(TCPSockSend , tcpMsg , strlen(tcpMsg) , 0) < 0)
    {
      writeLogErrno("Connection ended immediatelly. Cannot send request to server");
      stopRecvTCPThread();
      shutdownTcpConnection();
      return -1;
    }
    writeLog("tcp send: ", tcpMsg);
    if((readSize = recv(TCPSockSend , tcpMsg , 100 , 0)) < 0)
    {
      writeLogErrno("Connection ended immediatelly. Cannot receive from server");
      stopRecvTCPThread();
      shutdownTcpConnection();
      return -1;
    }
    writeLog("tcp recv: ", tcpMsg);
    tcpMsg[readSize] = '\0';
    if(strcmp(tcpMsg,"EXIT_OK") == 0)
    {
      stopRecvTCPThread();
      shutdownTcpConnection();
      return 0;
    }
    else
    {
      writeLogErrno("Connection ended immediatelly, server side error");
      return -1;
    }
    areUdpSocketsCreated = false;
    return 0;
}

void NetworkController::tcpRecv()
{
  while(recvTCPThread)
  {
    if(!isConnected)
    {
      printf("Brak polaczenia!\n");
      return;
    }
    int readSize;
    memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    while((readSize = recv(TCPSockRecv , tcpMsg , 100 , 0)) > 0)
    {
      writeLog("tcp recv: ", tcpMsg);
      tcpMsg[readSize] = '\0';
      if(strcmp(tcpMsg,"udp_start") == 0)
      {
        initializeSockaddrStruct(50002);
        strcpy(tcpMsg,"UDP_OK");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        writeLog("tcp send: ", tcpMsg);
        clientApp->startCallViaNet();
      }
      else if(strcmp(tcpMsg,"exit") == 0)
      {
        strcpy(tcpMsg, "EXIT_OK");
        send(TCPSockRecv , tcpMsg , strlen(tcpMsg), 0);
        writeLog("tcp send: ", tcpMsg);
        shutdownTcpConnection();
        clientApp->stopAudio();
        activSide = false;
        passivSide = false;
        return;
      }
      memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    }
  }
  return;
}

int NetworkController::createUdpSockets(int portRecv)
{
    udpConnection.myRecvSocket.port = portRecv;

    struct sockaddr_in stSockAddr;
    udpConnection.mySendSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpConnection.mySendSocketFD == -1)
    {
      writeLogErrno("Cannot create UDP socket");
      return -1;
    }

    udpConnection.myRecvSocket.socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpConnection.myRecvSocket.socket == -1)
    {
      writeLogErrno("Cannot create UDP socket");
      return -1;
    }

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddr.sin_port = htons(udpConnection.myRecvSocket.port);
    if(bind(udpConnection.myRecvSocket.socket,(struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      writeLogErrno("Cannot bind");
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
        writeLogErrno("udpSend error");
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
        writeLogErrno("udpRecv error");
      }
    }
    return 0;
}

int NetworkController::shutdownUdpConnection()
{
  if(udpConnection.mySendSocketFD > 0)
  {
    close(udpConnection.mySendSocketFD);
  }
  if(udpConnection.myRecvSocket.socket > 0)
  {
    close(udpConnection.myRecvSocket.socket);
  }
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


void NetworkController::stopRecvTCPThread()
{
    recvTCPThread = false;
}


void NetworkController::startSendUDPThread()
{
    sendUDPThread = true;
}
   

void NetworkController::startRecvUDPThread()
{
    recvUDPThread = true;
}


void NetworkController::startRecvTCPThread()
{
    recvTCPThread = true;
}


void NetworkController::writeLogErrno(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<", errno: "<<errno<<std::endl;
    }
}

void NetworkController::writeLog(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<std::endl;
    }
}

void NetworkController::writeLog(std::string message, int value)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<value<<std::endl;
    }
}  

void NetworkController::writeLog(std::string message, char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<msg<<std::endl;
    }
}  


//funkcja pomocnicza
void NetworkController::sampleFactory(BlockingQueue<Sample>& blockingQueue)
{
  int i = 0;
  float* message = (float*)malloc(FRAMES_PER_BUFFER*sizeof(float));
  while(++i < 100)
  {
    fflush(stdout);
    Sample sample;
    for(int j = 0; j<FRAMES_PER_BUFFER; j++)
    {
      message[j] = 0.00001 * j;
    }
    sample.setSample(message);
    blockingQueue.push(sample);
    sleep(1);
  }
  free(message);
}


