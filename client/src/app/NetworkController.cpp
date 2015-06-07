// SVC - Simple Voice Communicator 
// Kontroler połączenia sieciowego
// autor: Filip Gralewski - podstawa sieci
// autor: Beata Dobrzyńska - rozwinięcie funkcjonalności klienta


#include "NetworkController.hpp"
#include "ClientApp.hpp"


NetworkController::NetworkController(ClientApp* clientApp) : clientApp(clientApp)
{ 
    passivSide = false;
    activSide = false;
    isConnected = false;
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


int NetworkController::initTcpServer(int port)
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

    if(-1 == listen(connFD, 1))
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
  char tcpMsg[100];
  while(1)
  {
    int TCPSock = accept(connFD, NULL, NULL);
   
    if(TCPSock < 0)
    {
      writeLogErrno("tcpServer::Cannot accept");
      return -1;
    }
    else
    {
      writeLog("tcpServer::Connection accepted");
    }
    if(isConnected == true)
    {
      sendTCP(TCPSock, "BUSYY", "acceptTcpConnections");
      close(TCPSock);
      continue;
    }
    else
    {
      sendTCP(TCPSock, "READY", "acceptTcpConnections");
      TCPSockRecv = TCPSock;
    }
    recvTCP(TCPSockRecv, tcpMsg, 100, "acceptTcpConnections");
    std::string ip(tcpMsg, strlen(tcpMsg));
    if(activSide == false)
    {
      passivSide = true;
      if(clientApp->acceptCall(ip) == 0)
      {        
        sendTCP(TCPSockRecv, "CALL_ACCEPTED", "acceptTcpConnections");
        clientApp->commandThread();

        if(connectHost(port, &ip[0u], "ipv4") == 0)
        {
          isConnected = true;
          std::thread t1(&NetworkController::tcpRecvFromHost, this);
          t1.detach();
        }
        else
        {
          std::cout<<"Błąd połączenia"<<std::endl;
          shutdownTcpConnection();
        }
      }
      else
      {
        sendTCP(TCPSockRecv, "CALL_REFUSED", "acceptTcpConnections");
        shutdownTcpConnection();
        clientApp->commandThread();
        activSide = false;
        passivSide = false;
      }
      memset(&tcpMsg[0], 0, sizeof(tcpMsg));
    }
    else
    {  
      char* IP = &ip[0u];
      if(strcmp(IP, serverIPAddress) != 0)
      {
        shutdownTcpConnection();
        writeLog("Unauthorized connection");
        return -1;
      }
      else
      {
        isConnected = true;
        sendTCP(TCPSockRecv, "RETURN_ACCEPTED", "acceptTcpConnections");
        std::thread t1(&NetworkController::tcpRecvFromHost, this);
        t1.detach();
      }
    }
  }
  close(connFD);
  return 0;
}

int NetworkController::connectServer(int port, char* addr, char* login)
{
    char tcpMsg[100];
    struct sockaddr_in stSockAddr;
    int Res;
    TCPSockSend = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    TCPSockRecv = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    strcpy(serverIPAddress,addr);
    if(TCPSockSend == -1)
    {
      writeLogErrno("connectServer::Cannot create socket");
      return -1;
    }
    if(TCPSockRecv == -1)
    {
      writeLogErrno("connectServer::Cannot create socket");
      return -1;
    }
    
    memset(&stSockAddr, 0, sizeof(stSockAddr));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET, addr, &stSockAddr.sin_addr);
    if (0 > Res)
    {
      writeLog("connectServer::Invalid address family");
      close(TCPSockSend);
      return -2;
    }
    else if (0 == Res)
    {
      writeLog("connectServer::Invalid IP address");
      close(TCPSockSend);
      return -2;
    }
    writeLog("init connectServer::OK");
    if (-1 == connect(TCPSockSend, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)))
    {
      writeLogErrno("connectServer::Cannot connect first time");
      close(TCPSockSend);
      return -1;
    }
    else
    {
      writeLog("Connected to server first time");
    }
    std::string one("1");
    std::string firstLogin = one+login;
    sendTCP(TCPSockSend, &firstLogin[0u], "connectServer");
    recvTCP(TCPSockSend, tcpMsg, 10, "connectServer");
    if(strcmp(tcpMsg, "LOGIN_OK") == 0)
    {
      if (connect(TCPSockRecv, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
      {
        writeLogErrno("connectServer::Cannot connect second time");
        close(TCPSockSend);
        return -1;
      }
      else
      {
        writeLog("Connected to server first time");
      }
      std::string two("2");
      std::string secondLogin = two+login;
      sendTCP(TCPSockRecv, &secondLogin[0u], "connectServer");
      recvTCP(TCPSockRecv, tcpMsg, 9, "connectServer");
      unsigned int slen = sizeof(serverAddr);
      if(strcmp(tcpMsg, "CONNECTED") == 0)
      {
        int port1;
        sendTCP(TCPSockRecv, "PORTREQ", "connectServer");
        if(recv(TCPSockRecv, &port1, sizeof(port1), 0) == -1)
        {
          writeLogErrno("recv host addres");
          return -1;
        }
        /* Sending udp end point to server */
        if((holePunchSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) 
        {
          shutdownTcpConnection();
          return -1;
        }
        memset((char *) &serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port1);
        inet_aton(addr, &serverAddr.sin_addr);
        if(sendto(holePunchSocket, "0101010101", 10, 0, (struct sockaddr*)(&serverAddr), slen) == -1) 
        {
          shutdownTcpConnection();
          return -1;
        }
        isConnected = true;
        std::thread recvFromServerThread(&NetworkController::tcpRecvFromServer, this);
        recvFromServerThread.detach();
      }
      else
      {
        writeLog("server connection error");
        return -1;
      }
      return 0;
    }
    else
    {
      return -1;
    }
}

int NetworkController::connectHost(int port, char* addr, const char* addrFamily)
{
    char tcpMsg[100];
    int res;
    TCPSockSend = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(TCPSockSend == -1)
    {
      writeLogErrno("initTcpClient::Cannot create socket");
      return -1;
    }

    strcpy(serverIPAddress,addr);

    memset(&serverAddr, 0, sizeof(serverAddr));
   
    if(strcmp(addrFamily, "ipv4") == 0)
    {
      serverAddr.sin_family = AF_INET;
    }
    else if (strcmp(addrFamily, "ipv6") == 0)
    {
      serverAddr.sin_family = AF_INET6;
    }
    else
    {
      writeLog("initTcpClient::Invalid address family");
      close(TCPSockSend);
      return -2;
    }

    serverAddr.sin_port = htons(port);
    res = inet_pton(AF_INET, addr, &serverAddr.sin_addr);
    
    if(res < 0)
    {
      writeLog("initTcpClient::Invalid address family");
      close(TCPSockSend);
      return -2;
    }
    else if(res == 0)
    {
      writeLog("initTcpClient::Invalid IP address");
      close(TCPSockSend);
      return -2;
    }
    writeLog("initTcpClient::OK");

    int ret = connectTimeout(5);
    if(ret == -1)
    {
      writeLogErrno("connectHost::Cannot connect");
      close(TCPSockSend);
      return -1;
    }
    else if(ret == 1)
    {
      return -1; 
    }
    else
    {
      writeLog("Connected");
    }

    recvTCP(TCPSockSend, tcpMsg, 5, "connectHost");
    if(strcmp(tcpMsg, "BUSYY") == 0)
    {
      close(TCPSockSend);
      return 2;
    }
    getMyIp();
    sendTCP(TCPSockSend, myIPAddress, "connectHost");
    recvTCP(TCPSockSend, tcpMsg, 15, "connectHost");

    if(passivSide == false)
    {
      if(strcmp(tcpMsg, "CALL_REFUSED") == 0)
      {
        writeLog("connectHost::OK_refused");
        close(TCPSockSend);
        activSide = false;
        passivSide = false;
        return 1;
      }
      else if(strcmp(tcpMsg, "CALL_ACCEPTED") == 0)
      {
        writeLog("connectHost::OK_accepted");
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
        writeLog("connectHost::OK_return_connection");
        return 0;
      }
      else
      {
        writeLogErrno("connectHost::return_connection");
        return -1;
      }
    }
    return -1;
}

int NetworkController::connectTimeout(int sec)
{
  int res = -1; 
  int ret = -1;
  long arg; 
  struct timeval tv; 
  int valopt; 
  fd_set myset; 
  socklen_t lon; 
  // Set non-blocking 
  if((arg = fcntl(TCPSockSend, F_GETFL, NULL)) < 0) 
  { 
     writeLogErrno("fcntl");
     return -1;
  }

  arg |= O_NONBLOCK; 
  if(fcntl(TCPSockSend, F_SETFL, arg) < 0) 
  { 
     writeLogErrno("fcntl");
     return -1; 
  } 
  // Trying to connect with timeout 
  res = connect(TCPSockSend, (struct sockaddr *)&serverAddr, sizeof(serverAddr)); 
  if (res < 0) 
  { 
    if (errno == EINPROGRESS) 
    { 
      tv.tv_sec = sec; 
      tv.tv_usec = 0; 
      FD_ZERO(&myset); 
      FD_SET(TCPSockSend, &myset); 
      res = select(TCPSockSend+1, NULL, &myset, NULL, &tv); 
      switch(res)
      {
        case 0:
          std::cout<<"timed out!"<<std::endl;
          ret = 1;
          break;

        case -1:
          writeLogErrno("select");
          ret = -1;
          break;

        default:
          getsockopt(TCPSockSend, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
          if(valopt == 0)
          {
            ret = 0;
            break;
          }
          else
          {
            writeLogErrno("getsockopt");
            ret = -1;
            break;
          }
      }
    } 
    else 
    { 
      writeLogErrno("Connection error");
      return -1; 
    } 
  } 
  //set blocking mode
  if( (arg = fcntl(TCPSockSend, F_GETFL, NULL)) < 0) 
  { 
    writeLogErrno("fcntl");
    return -1;  
  } 
  arg &= (~O_NONBLOCK); 
  if( fcntl(TCPSockSend, F_SETFL, arg) < 0) 
  { 
    writeLogErrno("fcntl");
    return -1; 
  }
  //connected
  return ret; 
}


int NetworkController::shutdownTcpConnection()
{
    int ret1 = -1;
    int ret2 = -1;
    writeLog("Sockets before shutdown:");
    writeLogInt("TCPSockRecv: ", TCPSockRecv);
    writeLogInt("TCPSockSend: ", TCPSockSend);
    if(TCPSockSend > 0)
    {
      ret1 = shutdown(TCPSockSend, SHUT_RDWR);
      writeLogInt("shutdown TCPSockSend returned: ", ret1);
      TCPSockSend = 0;
      if( ret1 == -1 )
      {
        writeLogErrno("TCPSockSend failed");
        if(TCPSockSend > 0)
        {
          close(TCPSockSend);
          TCPSockSend = 0;
        };
      }
    }

    if(TCPSockRecv > 0)
    {
      ret2 = shutdown(TCPSockRecv, SHUT_RDWR);
      writeLogInt("shutdown TCPSockRecv returned: ", ret2);
      TCPSockRecv = 0;
      if( ret2 == -1 )
      {
        writeLogErrno("TCPSockRecv failed");
        if (TCPSockRecv > 0)
        {
          close(TCPSockRecv);
          TCPSockRecv = 0;
        };
      }
    }

    writeLog("Shutdown done!");
    return 0;
}



int NetworkController::startUDPNetTransmission()
{
    char tcpMsg[100];
    sendTCP(TCPSockSend, "udp_start", "startUDPNetTransmission");
    recvTCP(TCPSockSend, tcpMsg, 10, "startUDPNetTransmission");
    if(strcmp(tcpMsg, "UDP_OK") != 0)
    {
      writeLog("UDP error, server side!");
      return -1;
    }
    setServerAddr(50002);
    return 0;
}

int NetworkController::startUDPNatTransmission()
{
    char tcpMsg[100];
    sendTCP(TCPSockSend, "start_call", "startUDPNatTransmission");
    recvTCP(TCPSockSend, tcpMsg, 8, "startUDPNatTransmission");
    if(strcmp(tcpMsg, "STARTOK") == 0)
    {
      return 0;
    }
    else
    {
      return -1;
    }
}



void NetworkController::sendTCP(int sock, const char* msg, const char* function)
{
    if(send(sock, msg, strlen(msg), 0) == -1)
    {
      std::cout<<"network error"<<std::endl;
      writeLogErrno("sendTCP error", function);
    }
    else
    {
      char msg2[100];
      strcpy(msg2, msg);
      writeLog("sendTCP: ", msg2);
    }
}

int NetworkController::recvTCP(int sock, char *msg, int size, const char* function)
{
    int readSize = 0;
    memset(&msg[0], 0, 100*sizeof(char));
    readSize = recv(sock, msg, size, 0);
    if(readSize == -1)
    {
      std::cout<<"network error"<<std::endl;
      writeLogErrno("recvTCP error", function);
    }
    else if(readSize > 0)
    {
      msg[readSize] = '\0';
      writeLog("recvTCP: ", msg); 
    }
    return readSize;
}

void NetworkController::endNatConnection()
{
    char tcpMsg[100];
    sendTCP(TCPSockSend, "end_call", "endConnection");
    recvTCP(TCPSockSend, tcpMsg, 10, "endConnection");
    if(strcmp(tcpMsg,"ENDOK") == 0)
    {
      writeLog("connection ended succesfully");
    }
    else
    {
      writeLog("connection ended with errors");
    }
    sendKillPackets(1);
}

void NetworkController::endNetConnection()
{
    char tcpMsg[100];
    activSide = false;
    passivSide = false;
    sendTCP(TCPSockSend, "end_call", "endConnection");
    recvTCP(TCPSockSend, tcpMsg, 10, "endConnection");
    if(strcmp(tcpMsg,"ENDOK") == 0)
    {
      writeLog("connection ended succesfully");
    }
    else
    {
      writeLog("connection ended with errors");
    }
    sendKillPackets(0);
    shutdownTcpConnection();
}


void NetworkController::tcpRecvFromHost()
{
  char tcpMsg[100];
  while(recvTCP(TCPSockRecv , tcpMsg , 10, "tcpRecvFromHost") > 0)
  {
    if(strcmp(tcpMsg,"udp_start") == 0)
    {
      setServerAddr(50002);
      sendTCP(TCPSockRecv, "UDP_OK", "tcpRecvFromHost");
      clientApp->startCallViaNet();
    }
    else if(strcmp(tcpMsg,"end_call") == 0)
    {
      clientApp->stopNetTransmission();
      sendTCP(TCPSockRecv, "ENDOK", "tcpRecvFromHost");
      sendKillPackets(0);
      break;
    }
  }
  shutdownTcpConnection();
  isConnected = false;
  activSide = false;
  passivSide = false;
  return;
}

void NetworkController::tcpRecvFromServer()
{
  char tcpMsg[100];
  while(recvTCP(TCPSockRecv, tcpMsg, 8, "tcpRecvFromServer") > 0)
  {
    if(tcpMsg[0] == '_')
    {
      if(clientApp->acceptCall(tcpMsg) == 0)
      {        
        sendTCP(TCPSockRecv, "ACCEPTED", "tcpRecvFromServer");
        clientApp->commandThread();
      }
      else
      {
        sendTCP(TCPSockRecv, "REFUSEDD", "tcpRecvFromServer");
        clientApp->commandThread();
      }
    }
    else if(strcmp(tcpMsg, "ENDPOINT") == 0)
    {
      if(recvHolePunchData(TCPSockRecv) != 0)
      {
        exit(-1000);
      }
      std::cout<<"Gotowy aby rozpocząć rozmowę"<<std::endl;
    }
    else if(strcmp(tcpMsg, "ENDDCALL") == 0)
    {
      clientApp->stopNatTransmission();
      sendKillPackets(1);
      
    }
    else if(strcmp(tcpMsg, "STARTCAL") == 0)
    {
      clientApp->startCallViaNat();
    }
  }
  shutdownTcpConnection();
  isConnected = false;
  return;
}


int NetworkController::recvHolePunchData(int sock)
{
    int host;
    short port;
    if(recv(sock, &host, sizeof(host), 0) == -1)
    {
      writeLogErrno("recv host addres");
      return -1;
    }
    writeLogInt("host endpoint addres: ", host);
    if(recv(sock, &port, sizeof(port), 0) == -1)
    {
      writeLogErrno("recv host port");
      return -1;
    }
    writeLogInt("host endpoint port: ", port);
    setHolePunchAddr(host, port);
    return 0;
}

void NetworkController::logoutFromServer()
{
    sendTCP(TCPSockSend, "exit", "logoutFromServer");
}

void NetworkController::getUsersList()
{
    char tcpMsg[100];
    sendTCP(TCPSockSend, "list", "getUsersList");
    recvTCP(TCPSockSend, tcpMsg, 100, "getUsersList");
    if(strcmp(tcpMsg, "LIST") == 0)
    {
      std::cout<<"Użytkownicy na serwerze:"<<std::endl;
      while(strcmp(tcpMsg, "LISTEND") != 0)
      {
        sendTCP(TCPSockSend, "GETNEXT", "getUsersList");
        recvTCP(TCPSockSend, tcpMsg, 100, "getUsersList");
        if(strcmp(tcpMsg, "LISTEND") != 0)
        {
          std::cout<<tcpMsg<<std::endl;
        }
        else
        {
          std::cout<<"koniec listy"<<std::endl;
        }
      }
    }
    return;
}


int NetworkController::callPeerRequest(std::string login)
{
    char tcpMsg[100];
    std::string str = '_' + login;
    sendTCP(TCPSockSend, &str[0u], "callPeerRequest");
    recvTCP(TCPSockSend, tcpMsg, 8, "callPeerRequest");
    if(strcmp(tcpMsg, "ENDPOINT") == 0)
    {    
      if(recvHolePunchData(TCPSockSend) != 0)
      {
        shutdownTcpConnection();
        return -1;
      }
      return 0;
    }
    else if(strcmp(tcpMsg, "REFUSEDD") == 0)
    {
      return 1;
    }
    else if(strcmp(tcpMsg, "NOTAVAIL") == 0)
    {
      return 2;
    }
    return -1;
}

int NetworkController::createUdpSocket(int portRecv)
{
    bindedUDPSocket.port = portRecv;
    struct sockaddr_in stSockAddr;
    bindedUDPSocket.socket = socket(AF_INET, SOCK_DGRAM, 0);
    if(bindedUDPSocket.socket == -1)
    {
      writeLogErrno("Cannot create UDP socket");
      return -1;
    }
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    stSockAddr.sin_port = htons(bindedUDPSocket.port);
    if(bind(bindedUDPSocket.socket,
      (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1)
    {
      writeLogErrno("Cannot bind");
      return -1;
    }
    fflush(stdout);
    return 0;
}

void NetworkController::setHolePunchAddr(int host, short port)
{
    holePunchAddr.sin_family = AF_INET;
    holePunchAddr.sin_addr.s_addr = host; 
    holePunchAddr.sin_port = port;

}

void NetworkController::setServerAddr(int port)
{
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIPAddress , &serverAddr.sin_addr);
}

int NetworkController::udpSend(BlockingQueue<Sample>& blockingQueue, int mode)
{
    int sock;
    struct sockaddr_in addr;
    if(mode == 0)
    {
      sock = bindedUDPSocket.socket;
      addr = serverAddr;
    }
    else
    {
      sock = holePunchSocket;
      addr = holePunchAddr;
    }
    float buffer[FRAMES_PER_BUFFER];
    while(sendUDPThread)
    {
      memcpy(buffer,blockingQueue.pop().getSample(), sizeof(buffer));
      if(sendto(sock, buffer,FRAMES_PER_BUFFER * sizeof(float), 0, 
          (struct sockaddr*)&addr, sizeof(addr)) < 0)
      {
        writeLogErrno("udpSend error");
        return -1;
      } 
    }
    return 0;
}



int NetworkController::udpRecv(BlockingQueue<Sample>& blockingQueue, int mode)
{
    int sock;
    struct sockaddr_in addr;
    if(mode == 0)
    {
      sock = bindedUDPSocket.socket;
      addr = serverAddr;
    }
    else
    {
      sock = holePunchSocket;
      addr = holePunchAddr;
    }
    int readSize;
    Sample sample;
    float buffer[FRAMES_PER_BUFFER];
    unsigned int addrlen = sizeof(addr);
    while(recvUDPThread)
    {
      readSize = recvfrom(sock, buffer, FRAMES_PER_BUFFER * sizeof(float), 0, 
                  (struct sockaddr*)&addr, &addrlen);
      if(readSize > 0)
      {
        sample.setSample(buffer);
        blockingQueue.push(sample);
        fflush(stdout);
      }
      if(readSize < 0)
      {
        writeLogErrno("udpRecv error");
        return -1;
      }
    }
    return 0;
}



void NetworkController::sendKillPackets(int mode)
{
    int sock;
    struct sockaddr_in addr;
    float buffer[FRAMES_PER_BUFFER];
    for(int i=0; i<FRAMES_PER_BUFFER; i++)
    {
      buffer[i] = SAMPLE_SILENCE;
    }
    if(mode == 0)
    {
      sock = bindedUDPSocket.socket;
      addr = serverAddr;
    }
    else
    {
      sock = holePunchSocket;
      addr = holePunchAddr;
    }
    for(int i = 0; i < 5; i++)
    {
      sendto(sock, buffer,FRAMES_PER_BUFFER * sizeof(float), 0, 
          (struct sockaddr*)&addr, sizeof(addr));
    }
}


void NetworkController::closeUDPSockets()
{
  if(bindedUDPSocket.socket > 0)
  {
    close(bindedUDPSocket.socket);
  }
  if(holePunchSocket > 0)
  {
    close(holePunchSocket);
  }
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
    strcpy(myIPAddress, inet_ntop(AF_INET, &name.sin_addr, buffer, 15));
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


void NetworkController::writeLogErrno(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<", errno: "<<errno<<std::endl;
    }
}

void NetworkController::writeLogErrno(std::string message, char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<" in "<<msg<<", errno: "<<errno<<std::endl;
    }
}

void NetworkController::writeLogErrno(std::string message, const char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<" in "<<msg<<", errno: "<<errno<<std::endl;
    }
}


void NetworkController::writeLog(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<std::endl;
    }
}

void NetworkController::writeLogInt(std::string message, int value)
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




