#pragma once
#include <vector>
#include "types.h"

class IPacketRouter
{
public:
	virtual void routePacket(std::vector<uchar> packet) = 0;
};

