#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "PacketTransceiver.h"
#include "IPacketRouter.h"

class PacketRouter : public IPacketRouter
{
public:
	PacketRouter();
	int connectSource(PacketTransceiver& packetTransceiver);
	void send(int destinationID, std::vector<uint8_t> packet, uint8_t packetType);
	virtual void routePacket(int sourceID, std::vector<uint8_t> packet);
private:
	static int nextID;
	std::map<int, PacketTransceiver*> sourceMap;
	std::mutex sourceMapMutex;
};

