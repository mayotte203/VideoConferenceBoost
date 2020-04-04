#pragma once
#include <vector>
#include <mutex>
#include "PacketTransceiver.h"
#include "IPacketRouter.h"

class PacketRouter : public IPacketRouter
{
public:
	PacketRouter();
	void connectSource(PacketTransceiver& packetTransceiver);
	virtual void routePacket(const PacketTransceiver& packetTransceiver, std::vector<uint8_t> packet);
private:
	std::list<PacketTransceiver*> packetTransceiverList;
	std::mutex sourceMapMutex;
};

