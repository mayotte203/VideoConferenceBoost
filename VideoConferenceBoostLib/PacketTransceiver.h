#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include "IPacketRouter.h"
#include "ExceptionTransporter.h"

class PacketTransceiver
{
public:
	PacketTransceiver();
	PacketTransceiver(PacketTransceiver&&);
	PacketTransceiver(const PacketTransceiver&) = delete;
	~PacketTransceiver();
	PacketTransceiver& operator= (const PacketTransceiver&) = delete;
	PacketTransceiver& operator= (PacketTransceiver&&);

	void connect(boost::asio::ip::tcp::socket&& socket);
	void disconnect();
	void sendPacket(std::vector<uint8_t> packet);
	void connectRouter(IPacketRouter& packetRouter);
	bool isConnected();
private:
	static constexpr size_t RECEVIER_BUF_SIZE = 512 * 1024;
	boost::asio::ip::tcp::socket* socket = nullptr;

	IPacketRouter* packetRouter = nullptr;
	std::condition_variable receiverCondition;
	std::condition_variable senderCondition;
	std::thread senderThread;
	std::thread receiverThread;
	std::mutex senderMutex;
	std::mutex receiverMutex;
	std::queue<std::vector<uint8_t>> senderQueue;
	std::mutex senderQueueMutex;
	std::condition_variable senderQueueCondition;
	bool connected = false;
	void senderThreadFunction();
	void receiverThreadFunction();
};

