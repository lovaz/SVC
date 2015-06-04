#ifndef THREADSAFEMAP_HPP
#define THREADSAFEMAP_HPP

#include <map>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <list>
#include <iomanip> 



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



class ThreadSafeMap
{
private:
	std::map<std::string, SingleClientConnection> safeMap;
	std::mutex mapMutex;
	std::condition_variable mapCond;
public:
	int insert(std::string key, SingleClientConnection* item)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		if(safeMap.find(key) == safeMap.end())
		{
			safeMap.insert(std::pair<std::string, SingleClientConnection>(key, *item));
			mlock.unlock();
			mapCond.notify_one();
			return 0;
		}
		return -1;
	}

	int erase(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		if(safeMap.find(key) == safeMap.end())
		{
			return -1;
		}
		safeMap.erase(key);
		mlock.unlock();
		mapCond.notify_one();
		return 0;
	}
	int contains(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			mlock.unlock();
			mapCond.notify_one();
			return -1;
		}
		else
		{
			mlock.unlock();
			mapCond.notify_one();
			return 0;
		}
	}

	int update(std::string key, int TCPSockSend)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter != safeMap.end())
		{
			iter->second.TCPSockSend = TCPSockSend;
			mlock.unlock();
			mapCond.notify_one();
			return 0;
		}
		else
		{
			mlock.unlock();
			mapCond.notify_one();
			return -1;
		}
	}

	int update(std::string key, int host, short port)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter != safeMap.end())
		{
			iter->second.clientEndPoint.host = host;
			iter->second.clientEndPoint.port = port;
			mlock.unlock();
			mapCond.notify_one();
			return 0;
		}
		else
		{
			mlock.unlock();
			mapCond.notify_one();
			return -1;
		}
	}

	void printMap()
	{
		struct sockaddr_in temp;
		std::unique_lock<std::mutex> mlock(mapMutex);
		std::cout.width(15);
		std::cout<<std::left<<"User";
		std::cout.width(16);
		std::cout<<std::left<<"IP";
		std::cout.width(6);
		std::cout<<std::left<<"Port";
		std::cout.width(6);
		std::cout<<std::left<<"Busy"<<std::endl;
		for(auto iter = safeMap.begin(); iter != safeMap.end(); iter++)
		{
			temp.sin_addr.s_addr = iter->second.clientEndPoint.host;
			std::cout.width(15);
			std::cout<<std::left<<iter->first;
			std::cout.width(16);
			std::cout<<std::left<<inet_ntoa(temp.sin_addr);
			std::cout.width(6);
			std::cout<<std::left<<ntohs(iter->second.clientEndPoint.port);
			std::cout.width(6);
			std::cout<<std::left<<iter->second.busy<<std::endl;
		}
		mlock.unlock();
		mapCond.notify_one();
	}

	int getRecvSock(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		int Socket = iter->second.TCPSockRecv;
		mlock.unlock();
		mapCond.notify_one();
		return Socket;		
	}

	int getSendSock(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		int Socket = iter->second.TCPSockSend;
		mlock.unlock();
		mapCond.notify_one();
		return Socket;		
	}

	void getAddr(ClientEndPoint &endPoint, std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			return;
		}
		endPoint.host = iter->second.clientEndPoint.host;
		endPoint.port = iter->second.clientEndPoint.port;
		mlock.unlock();
		mapCond.notify_one();
	}

	bool getBusy(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			return true;
		}
		bool ret = iter->second.busy;
		mlock.unlock();
		mapCond.notify_one();
		return ret;
	}

	void setBusy(std::string key, bool busy)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			return;
		}
		iter->second.busy = busy;
		mlock.unlock();
		mapCond.notify_one();
	}

	void setPartner(std::string key, std::string partner)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			return;
		}
		iter->second.callPartner = partner;
		mlock.unlock();
		mapCond.notify_one();
	}

	std::string getPartner(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		auto iter = safeMap.find(key);
		if(iter == safeMap.end())
		{
			return key;
		}
		std::string partner = iter->second.callPartner;
		mlock.unlock();
		mapCond.notify_one();
		return partner;
	}

	std::list<std::pair<std::string, bool>> getUsers()
	{
		std::list<std::pair<std::string, bool>> usersList;
		for(auto iter = safeMap.begin(); iter != safeMap.end(); iter++)
		{
			usersList.push_front(std::pair<std::string, bool>(iter->first, iter->second.busy));
		}
		return usersList;
	}

};

#endif