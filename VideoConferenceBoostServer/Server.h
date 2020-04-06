#pragma once
#include <boost/asio.hpp>
#include <PacketTransceiver.h>
#include "PacketRouter.h"

class Server
{
public:
	Server(unsigned short serverPort);
	Server() = delete;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	Server& operator= (const Server&) = delete;
	Server& operator= (Server&&) = delete;
	~Server();
	void start();
	void stop();
private:
	constexpr static size_t CLIENT_MAX_COUNT = 2;
	unsigned short port;
	boost::asio::io_service ioservice;
	PacketRouter packetRouter;
	PacketTransceiver packetTransceiverArr[CLIENT_MAX_COUNT];
	std::condition_variable acceptorCondtion;
	std::thread acceptorThread;
	void acceptorFunction();
	bool working = true;
};

