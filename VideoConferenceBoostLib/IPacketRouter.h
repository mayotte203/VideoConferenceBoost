#pragma once
#include <vector>
class PacketTransceiver;

class IPacketRouter
{
public:
	virtual void routePacket(const PacketTransceiver& packetTransceiver, std::vector<uint8_t> packet) = 0;
};

