// SVC - Simple Voice Communicator 
// kontroler serwera dla połączeń przez NAT
// autor: Filip Gralewski - podstawa sieciowa
// autor: Marcin Frankowski - logika serwera

#include "ServerController.hpp"



ServerController::ServerController()
{ 
    networkLog.open("network.log", std::ios::out | std::ios::trunc);
    if(networkLog.good() != true)
    {
      std::cout << "Nie udało się otworzyć pliku network.log"<<std::endl;
    }
}


ServerController::~ServerController()
{
    if(networkLog.good() == true)
    {
      networkLog.close();
    }
}


int ServerController::initTcpServer(int port)
{
    struct sockaddr_in stSockAddr;
    connFD = socket(PF_INET, SOCK_STREAM, 0);

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

    if(-1 == listen(connFD, 100))
    {
      writeLogErrno("initTcpServer::Cannot listen");
      close(connFD);
      return -1;
    }
    writeLog("initTcpServer::OK");
    return 0;
}

void ServerController::singleConnectionHandler(std::string login)
{
  char tcpMsg[100];
  int receiverSocket = -1;
  std::string receiverLogin;
  int TCPSockRecv = clientsMap.getRecvSock(login);
  int TCPSockSend = clientsMap.getSendSock(login);
  while(recvTCP(TCPSockRecv , tcpMsg , 100, "singleConnectionHandler") > 0)
  {
    if(tcpMsg[0] == '_')
    {
      std::string tmp(tcpMsg+1);
      receiverLogin = tmp;
      int ret1 = clientsMap.contains(receiverLogin);
      bool ret2 = clientsMap.getBusy(receiverLogin);
      if(ret1 == 0 && ret2 == false)
      {
        clientsMap.setBusy(receiverLogin, true);
        clientsMap.setBusy(login, true);
        clientsMap.setPartner(login, receiverLogin);
        clientsMap.setPartner(receiverLogin, login);
        receiverSocket = clientsMap.getSendSock(receiverLogin);
        login = '_' + login;
        sendTCP(receiverSocket, &login[0u], "singleConnectionHandler");
        recvTCP(receiverSocket, tcpMsg, 10, "singleConnectionHandler");
        if(strcmp(tcpMsg, "ACCEPTED") == 0)
        {
          sendTCP(TCPSockRecv, "ENDPOINT", "singleConnectionHandler");
          ClientEndPoint cep;
          clientsMap.getAddr(cep, receiverLogin);
          int host = cep.host;
          short port = cep.port;
          send(TCPSockRecv, &host, sizeof(host), 0);
          send(TCPSockRecv, &port, sizeof(port), 0);
          sendTCP(receiverSocket, "ENDPOINT", "singleConnectionHandler");
          clientsMap.getAddr(cep, login.erase(0,1));
          host = cep.host;
          port = cep.port;
          send(receiverSocket, &host, sizeof(host), 0);
          send(receiverSocket, &port, sizeof(port), 0);
        }
        else
        {
          login = login.erase(0,1);
          clientsMap.setBusy(receiverLogin, false);
          clientsMap.setBusy(login, false);
          clientsMap.setPartner(login, login);
          clientsMap.setPartner(receiverLogin, receiverLogin);
          sendTCP(TCPSockRecv, "REFUSEDD", "singleConnectionHandler");
        }
      }
      else
      {
        sendTCP(TCPSockRecv, "NOTAVAIL", "singleConnectionHandler");
      }
    }
    else if(strcmp(tcpMsg,"start_call") == 0)
    {
      std::string partner;
      if((partner = clientsMap.getPartner(login)) != login)
      {
        sendTCP(clientsMap.getSendSock(partner), "STARTCAL", "singleConnectionHandler");
        sendTCP(TCPSockRecv, "STARTOK", "singleConnectionHandler");
      }
      else
      {
        sendTCP(TCPSockRecv, "NOTAVAIL", "singleConnectionHandler");
        clientsMap.setBusy(login, false);
      }
    }
    else if(strcmp(tcpMsg,"end_call") == 0)
    {
      std::string partner;
      if((partner = clientsMap.getPartner(login)) != login)
      {
        sendTCP(clientsMap.getSendSock(partner), "ENDDCALL", "singleConnectionHandler");
        sendTCP(TCPSockRecv, "ENDOK", "singleConnectionHandler");
        clientsMap.setBusy(login, false);
        clientsMap.setBusy(partner, false);
        clientsMap.setPartner(login, login);
        clientsMap.setPartner(receiverLogin, receiverLogin);
      }
      else
      {
        sendTCP(TCPSockRecv, "NOTAVAIL", "singleConnectionHandler");
        clientsMap.setBusy(login, false);
      }
    }
    else if(strcmp(tcpMsg, "exit") == 0)
    {
      close(TCPSockSend);
      close(TCPSockRecv);
    }
    else if(strcmp(tcpMsg, "list") == 0)
    {
      std::list<std::pair<std::string, bool>> usersList = clientsMap.getUsers();
      sendTCP(TCPSockRecv, "LIST", "singleConnectionHandler");
      for(auto iter = usersList.begin(); iter != usersList.end(); iter ++)
      {
        recvTCP(TCPSockRecv, tcpMsg,7, "singleConnectionHandler");
        sendTCP(TCPSockRecv, &iter->first[0u], "singleConnectionHandler");
      }
      recvTCP(TCPSockRecv, tcpMsg, 7, "singleConnectionHandler");
      sendTCP(TCPSockRecv, "LISTEND", "singleConnectionHandler");
    }
  }
  std::string partner;
  if((partner = clientsMap.getPartner(login)) != login)
  {
    sendTCP(clientsMap.getSendSock(partner), "ENDDCALL", "singleConnectionHandler");
    clientsMap.setBusy(partner, false);
  }
  clientsMap.erase(login);
  std::cout<<"User "<<login<<" logged out"<<std::endl;
  close(TCPSockSend);
  close(TCPSockRecv);
  return;
}


int ServerController::acceptTcpConnections(int port)
{
  unsigned int slen=sizeof(clientAddr);
  char tcpMsg[100];
  char buf[512];
  while(1)
  {
    TCPSocket = accept(connFD, NULL, NULL);
    if(TCPSocket < 0)
    {
      writeLogErrno("tcpServer::Cannot accept");
      return -1;
    }
    else
    {
      writeLog("tcpServer::Connection accepted");
    }
    recvTCP(TCPSocket, tcpMsg, 100, "acceptTcpConnections");
    std::string login(tcpMsg);
    if(login[0] == '1')
    {
      login.erase(0,1);
      if(clientsMap.insert(login, new SingleClientConnection(TCPSocket)) == -1)
      {
        sendTCP(TCPSocket, "LOGIN_TAKEN", "acceptTcpConnections");
        close(TCPSocket);
      }
      else
      {
        sendTCP(TCPSocket, "LOGIN_OK", "acceptTcpConnections");
        clientsMap.setPartner(login, login);
      }
    }
    if(login[0] == '2')
    {
      if(clientsMap.contains(login.erase(0,1)) == 0)
      {
        int udpPort = 0;
        clientsMap.update(login, TCPSocket);

        if((udpPort = fillUDPConnectionData()) < 0)
        {
          sendTCP(TCPSocket, "SERVERROR", "acceptTcpConnections");
          break; 
        }

        sendTCP(TCPSocket, "CONNECTED", "acceptTcpConnections");
        recvTCP(TCPSocket, tcpMsg, 7,"acceptTcpConnections");

        if(strcmp(tcpMsg, "PORTREQ") == 0)
        {
          if(send(TCPSocket, &udpPort, sizeof(udpPort), 0) == -1)
          {
            break;
          } 
        }
        if(recvfrom(UDPSocket, buf, 512, 0, (struct sockaddr*)(&clientAddr), &slen)==-1) 
        {
          sendTCP(TCPSocket, "SERVERROR", "acceptTcpConnections");
          break;
        }
        clientsMap.update(login, clientAddr.sin_addr.s_addr, clientAddr.sin_port);
        threadList.emplace_back(&ServerController::singleConnectionHandler, this, login);
        close(UDPSocket);
        std::cout<<"User "<<login<<" has joined server"<<std::endl;
      }
      else
      {
        sendTCP(TCPSocket, "SERVERROR", "acceptTcpConnections");
        close(TCPSocket);
      }
    }
  } 
  for(std::thread &t : threadList)
  {
    t.join();
  }
  close(connFD);
  return 0;
}

int ServerController::fillUDPConnectionData()
{
  int port = 40999;
  
  if((UDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) 
  {
    writeLogErrno("create UDP Socket");
    return -1;
  }
  memset((char *) &myAddr, 0, sizeof(myAddr));
  memset((char *) &clientAddr, 0, sizeof(clientAddr));
  int ret = -1;
  while(ret == -1)
  {
    port++;
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(port);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(UDPSocket, (struct sockaddr*)(&myAddr), sizeof(myAddr));
    if(port > 42000)
    {
      writeLogErrno("Cannot bind udp socket");
      return -1;
    }
  }
  return port;
}


void ServerController::sendTCP(int sock, const char* msg, const char* function)
{
    if(send(sock, msg, strlen(msg), 0) == -1)
    {
      writeLogErrno("sendTCP error", function);
    }
    else
    {
      char msg2[100];
      strcpy(msg2, msg);
      writeLog("sendTCP: ", msg2);
    }
}

int ServerController::recvTCP(int sock, char *msg, int size, const char* function)
{
    int readSize = 0;
    memset(&msg[0], 0, 100*sizeof(char));
    readSize = recv(sock, msg, size, 0);
    if(readSize == -1)
    {
      writeLogErrno("recvTCP error", function);
    }
    else if(readSize > 0)
    {
      msg[readSize] = '\0';
      writeLog("recvTCP: ", msg); 
    }
    return readSize;
}

void ServerController::cleanSockets()
{
    clientsMap.closeSockets();
    if(connFD > 0)
    {
      close(connFD);
    }
}

void ServerController::printUsers()
{
    clientsMap.printMap();
}


void ServerController::writeLogErrno(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<", errno: "<<errno<<std::endl;
    }
}

void ServerController::writeLogErrno(std::string message, char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<" in "<<msg<<", errno: "<<errno<<std::endl;
    }
}

void ServerController::writeLogErrno(std::string message, const char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<" in "<<msg<<", errno: "<<errno<<std::endl;
    }
}


void ServerController::writeLog(std::string message)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<std::endl;
    }
}

void ServerController::writeLogInt(std::string message, int value)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<value<<std::endl;
    }
}  

void ServerController::writeLog(std::string message, char* msg)
{
    if(networkLog.good() == true)
    {
      networkLog<<message<<msg<<std::endl;
    }
}  
