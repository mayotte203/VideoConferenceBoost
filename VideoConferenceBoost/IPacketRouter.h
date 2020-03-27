#pragma once
#include <vector>
#include "types.h"

class IPacketRouter
{
public:
	virtual void routePacket(std::vector<uint8_t> packet) = 0;
};

