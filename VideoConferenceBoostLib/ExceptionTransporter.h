#pragma once
#include <exception>
#include <utility>
namespace ExceptionTransporter
{
	void transportException(void* invoker, std::exception exception);
	std::pair<void*, std::exception> retrieveException();
	bool isEmpty();
};