#include "ExceptionTransporter.h"
namespace ExceptionTransporter
{
	std::queue<std::pair<void*, std::exception>> exceptionQueue;
	std::mutex exceptionQueueMutex;

	void ExceptionTransporter::throwException(void* invoker, std::exception exception)
	{
		std::scoped_lock lock(exceptionQueueMutex);
		exceptionQueue.push(std::pair(invoker, exception));
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

}