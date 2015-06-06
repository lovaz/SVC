// SVC - Simple Voice Communicator 
// Kolejka blokująca do komunikacji między wątkami
// autor: Marcin Frankowski

#ifndef BLOCKINGQUEUE_HPP
#define BLOCKINGQUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class BlockingQueue
{
private:
	std::queue<T> blockingQueue;
	std::mutex bqMutex;
	std::condition_variable bqCond;
public:
	void push(T &item)
	{
		std::unique_lock<std::mutex> mlock(bqMutex);
		blockingQueue.push(item);
		mlock.unlock();
		bqCond.notify_one();
	}
	T pop()
	{
		std::unique_lock<std::mutex> mlock(bqMutex);
		while (blockingQueue.empty())
		{
		bqCond.wait(mlock);
		}
		auto item = blockingQueue.front();
		blockingQueue.pop();
		return item;
	}
	void clear()
	{
		std::unique_lock<std::mutex> mlock(bqMutex);
		std::queue<T> empty;
		std::swap(blockingQueue, empty);
		mlock.unlock();
		bqCond.notify_one();
	}
};

#endif