// SVC - Simple Voice Communicator 
// bezpieczna wątkowo mapa klientów
// autor: Marcin Frankowski

#ifndef CLIENTSMAP_HPP
#define CLIENTSMAP_HPP

#include <map>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <list>
#include <iomanip> 
#include <arpa/inet.h>
#include <unistd.h>




typedef struct
{
   int host;
   short port;
}ClientEndPoint;


class SingleClientConnection
{
public:
   SingleClientConnection(int TCPSockRecv)
   {
   	 this->TCPSockRecv = TCPSockRecv;
   	 busy = false;
   }
   int TCPSockSend;
   int TCPSockRecv;
   bool busy;
   ClientEndPoint clientEndPoint;
   std::string callPartner;
};



class ClientsMap
{
private:
	std::map<std::string, SingleClientConnection> safeMap;
	std::mutex mapMutex;
	std::condition_variable mapCond;
public:
	int insert(std::string key, SingleClientConnection* item);
	int erase(std::string key);
	int contains(std::string key);
	int update(std::string key, int TCPSockSend);
	int update(std::string key, int host, short port);
	int getRecvSock(std::string key);
	int getSendSock(std::string key);
	bool getBusy(std::string key);
	void getAddr(ClientEndPoint &endPoint, std::string key);
	void setBusy(std::string key, bool busy);
	void setPartner(std::string key, std::string partner);
	void printMap();
	void closeSockets();
	std::string getPartner(std::string key);
	std::list<std::pair<std::string, bool>> getUsers();
};

#endif