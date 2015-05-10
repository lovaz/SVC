#include "BlockingQueue.hpp"

template <typename T>
T BlockingQueue<T>::pop()
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

template <typename T>
void BlockingQueue<T>::push(T &item)
{
	std::unique_lock<std::mutex> mlock(bqMutex);
	blockingQueue.push(item);
	mlock.unlock();
	bqCond.notify_one();
}


