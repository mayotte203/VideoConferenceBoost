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
	//void send(int destinationID, std::vector<uint8_t> packet, uint8_t packetType);
	virtual void routePacket(const PacketTransceiver& packetTransceiver, std::vector<uint8_t> packet);
private:
	std::list<PacketTransceiver*> packetTransceiverList;
	std::mutex sourceMapMutex;
};

