#ifndef THREADSAFEMAP_HPP
#define THREADSAFEMAP_HPP

#include <map>
#include <mutex>
#include <condition_variable>
#include <iostream>

template <typename T>
class ThreadSafeMap
{
private:
	std::map<std::string, T> safeMap;
	std::mutex mapMutex;
	std::condition_variable mapCond;
public:
	int insert(std::string key, T &item)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		if(safeMap.find(key) == safeMap.end())
		{
			safeMap.insert(std::pair<std::string, T>(key, item));
			mlock.unlock();
			mapCond.notify_one();
			return 0;
		}
		return -1;
	}
	int delete_(std::string key)
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
	T find(std::string key)
	{
		std::unique_lock<std::mutex> mlock(mapMutex);
		T t = safeMap[key];
		mlock.unlock();
		mapCond.notify_one();
		return t;
	}
};

#endif