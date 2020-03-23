#pragma once
#include <exception>
#include <queue>
#include <mutex>

namespace ExceptionTransporter
{
	enum class Invoker{SenderThread, ReceiverThread};
	void throwException(Invoker invoker, std::exception exception);
	std::pair<Invoker, std::exception> retrieveException();
	bool isEmpty();
};

