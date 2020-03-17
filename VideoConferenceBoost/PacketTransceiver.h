#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include "types.h"

class PacketTransceiver
{
public:
	PacketTransceiver() = delete;
	PacketTransceiver(boost::asio::ip::tcp::socket *socket);
	void sendPacket(std::vector<uchar> packet);
	std::vector<uchar> receivePacket();
	bool isPacketReady();
	std::condition_variable* getReceiveCondVar();

private:
	static constexpr size_t RECEVIER_BUF_SIZE = 512 * 1024;
	boost::asio::ip::tcp::socket *socket;
	std::condition_variable receiveCondition;
	std::condition_variable senderCondition;
	std::thread senderThread;
	std::thread receiverThread;
	std::mutex senderMutex;
	std::mutex receiverMutex;
	std::vector<uchar> receivedPacket;
	std::queue<std::vector<uchar>> receiverQueue;
	std::queue<std::vector<uchar>> senderQueue;
	void senderThreadFunction();
	void receiverThreadFunction();
};

