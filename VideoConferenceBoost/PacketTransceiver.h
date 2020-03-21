#pragma once
#include <boost/asio.hpp>
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
	PacketTransceiver(boost::asio::ip::tcp::socket &socket);
	void sendPacket(std::vector<uchar> packet);
	std::vector<uchar> receivePacket();
	bool isPacketReady();
	std::condition_variable* getReceiveCondVar();
	bool isReceiving();
	bool isSending();
	void setReceiving(bool isReceiving);
	void setSending(bool isSending);

private:
	static constexpr size_t RECEVIER_BUF_SIZE = 512 * 1024;
	boost::asio::ip::tcp::socket *socket;
	std::condition_variable receiveCondition;
	std::condition_variable sendCondition;
	std::thread senderThread;
	std::thread receiverThread;
	std::mutex senderMutex;
	std::mutex receiverMutex;
	std::queue<std::vector<uchar>> receiverQueue;
	std::queue<std::vector<uchar>> senderQueue;
	bool sending = false;
	bool receiving = false;
	void senderThreadFunction();
	void receiverThreadFunction();
};

