#pragma once
#include <vector>

class IPacketEndpoint
{
public:
	virtual void handlePacket(const std::vector<uint8_t> packet, uint8_t packetType) = 0;
};

