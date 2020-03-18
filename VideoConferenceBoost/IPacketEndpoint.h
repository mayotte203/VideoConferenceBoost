#pragma once
#include <vector>
#include "types.h"

class IPacketEndpoint
{
public:
	virtual void handlePacket(const std::vector<uchar> packet, uchar packetType) = 0;
};

