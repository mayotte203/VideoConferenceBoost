#include "PacketRouter.h"
#include <algorithm>

PacketRouter::PacketRouter(PacketTransceiver& packetTr)
{
	packetTransceiver = &packetTr;
    packetTransceiver->connectRouter(*this);
}

void PacketRouter::connect(IPacketEndpoint& packetEndpoint, uchar packetType)
{
	std::scoped_lock lock(endpointMapMutex);
    endpointMap[packetType] = &packetEndpoint;//maybe check if already occupied
}
//maybe there is a lot to think about... 
void PacketRouter::disconnect(IPacketEndpoint& packetEndpoint, uchar packetType)
{
	std::scoped_lock lock(endpointMapMutex);
    if (endpointMap[packetType] == &packetEndpoint)
    {
        endpointMap[packetType] = nullptr;
    }
}

void PacketRouter::send(std::vector<uchar> packet, uchar packetType)
{
	packet.push_back(packetType);
	packetTransceiver->sendPacket(std::move(packet));
}

void PacketRouter::routePacket(std::vector<uchar> packet)
{
    uchar packetType = packet.back();
    packet.pop_back();
    endpointMapMutex.lock();
    if (endpointMap[packetType] != nullptr)
    {
        endpointMap[packetType]->handlePacket(packet, packetType);
    }
    endpointMapMutex.unlock();
}