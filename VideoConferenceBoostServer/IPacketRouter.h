#pragma once
#include <vector>

class IPacketRouter
{
public:
	virtual void routePacket(int sourceID, std::vector<uint8_t> packet) = 0;
};

