#pragma once
#include <exception>
#include <queue>
#include <mutex>

namespace ExceptionTransporter
{
	enum class Invoker{SenderThread, ReceiverThread};
	void throwException(std::exception exception, Invoker invoker);
	std::pair<Invoker, std::exception> retrieveException();
	bool isEmpty();
};

