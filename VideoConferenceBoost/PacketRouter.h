#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "PacketTransceiver.h"
#include "IPacketEndpoint.h"
#include "types.h"

class PacketRouter
{
public:
	PacketRouter() = delete;
	PacketRouter(PacketTransceiver& packetTr);
	void connect(IPacketEndpoint& packetEndpoint, uchar packetType);
	void disconnect(IPacketEndpoint& packetEndpoint, uchar packetType);
	void send(std::vector<uchar> packet, uchar packetType);
private:
	std::map<uchar, std::queue<std::vector<uchar>>> receivedPacketMap;
	std::map<uchar, IPacketEndpoint*> endpointMap;
	std::mutex endpointMapMutex;
	PacketTransceiver* packetTransceiver;
	std::thread packetRouterThread;
	void packetRouterThreadFunction();
};

