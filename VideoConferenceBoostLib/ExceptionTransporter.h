#pragma once
#include <exception>
#include <condition_variable>
#include <utility>
namespace ExceptionTransporter
{
	void transportException(void* invoker, std::exception exception);
	std::pair<void*, std::exception> retrieveException();
	bool isEmpty();
	std::condition_variable* getReadyCondition();
};