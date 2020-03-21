#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include "types.h"
#include "IPacketRouter.h"

class PacketTransceiver
{
public:
	PacketTransceiver() = delete;
	PacketTransceiver(boost::asio::ip::tcp::socket &socket);//
	void sendPacket(std::vector<uchar> packet);
	bool isReceiving();
	bool isSending();
	void setReceiving(bool isReceiving);
	void setSending(bool isSending);
	void connectRouter(IPacketRouter& packetRouter);
	void connect(boost::asio::ip::address address);//
	void disconnect();//
private:
	static constexpr size_t RECEVIER_BUF_SIZE = 512 * 1024;
	boost::asio::ip::tcp::socket* socket;
	IPacketRouter* packetRouter = nullptr;
	std::condition_variable receiverCondition;
	std::condition_variable senderCondition;
	std::thread senderThread;
	std::thread receiverThread;
	std::mutex senderMutex;
	std::mutex receiverMutex;
	std::queue<std::vector<uchar>> senderQueue;
	std::mutex senderQueueMutex;
	std::condition_variable senderQueueCondition;
	bool sending = false;
	bool receiving = false;
	void senderThreadFunction();
	void receiverThreadFunction();
};

