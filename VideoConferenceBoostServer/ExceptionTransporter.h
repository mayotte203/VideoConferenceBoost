#pragma once
#include <exception>
#include <queue>
#include <mutex>

namespace ExceptionTransporter
{
	void throwException(void* invoker, std::exception exception);
	std::pair<void*, std::exception> retrieveException();
	bool isEmpty();
};

