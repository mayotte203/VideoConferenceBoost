#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "PacketTransceiver.h"
#include "IPacketEndpoint.h"
#include "types.h"
#include "IPacketRouter.h"

class PacketRouter : public IPacketRouter
{
public:
	PacketRouter() = delete;
	PacketRouter(PacketTransceiver& packetTr);
	void connect(IPacketEndpoint& packetEndpoint, uchar packetType);
	void disconnect(IPacketEndpoint& packetEndpoint, uchar packetType);
	void send(std::vector<uchar> packet, uchar packetType);
	virtual void routePacket(std::vector<uchar> packet);
private:
	std::map<uchar, IPacketEndpoint*> endpointMap;
	std::mutex endpointMapMutex;
	PacketTransceiver* packetTransceiver;
};

