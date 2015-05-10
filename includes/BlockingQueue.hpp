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
	void push(T &item);
	T pop();
};

#endif