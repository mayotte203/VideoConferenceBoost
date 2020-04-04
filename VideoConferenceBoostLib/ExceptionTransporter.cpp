#include "ExceptionTransporter.h"
#include <queue>
#include <mutex>

namespace ExceptionTransporter
{
	std::queue<std::pair<void*, std::exception>> exceptionQueue;
	std::mutex exceptionQueueMutex;
	std::condition_variable exceptionReadyCondition;

	void ExceptionTransporter::transportException(void* invoker, std::exception exception)
	{
		std::scoped_lock lock(exceptionQueueMutex);
		exceptionQueue.push(std::pair(invoker, exception));
		exceptionReadyCondition.notify_all();
	}

	std::pair<void*, std::exception> ExceptionTransporter::retrieveException()
	{
		std::scoped_lock lock(exceptionQueueMutex);
		auto result = std::pair(exceptionQueue.back().first, exceptionQueue.back().second);
		exceptionQueue.pop();
		return result;
	}

	bool ExceptionTransporter::isEmpty()
	{
		std::scoped_lock lock(exceptionQueueMutex);
		return exceptionQueue.empty();
	}
	std::condition_variable* getReadyCondition()
	{
		return &exceptionReadyCondition;
	}
}