#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "PacketTransceiver.h"
#include "IPacketEndpoint.h"
#include "IPacketRouter.h"

class PacketRouter : public IPacketRouter
{
public:
	PacketRouter() = delete;
	PacketRouter(PacketTransceiver& packetTr);
	void connect(IPacketEndpoint& packetEndpoint, uint8_t packetType);
	void send(std::vector<uint8_t> packet, uint8_t packetType);
	virtual void routePacket(const PacketTransceiver& packetTransceiver, std::vector<uint8_t> packet);
private:
	std::map<uint8_t, IPacketEndpoint*> endpointMap;
	std::mutex endpointMapMutex;
	PacketTransceiver* packetTransceiver;
};

