#include "PacketRouter.h"
#include <algorithm>

PacketRouter::PacketRouter(PacketTransceiver& packetTr)
{
	packetTransceiver = &packetTr;
    packetTransceiver->connectRouter(*this);
}

void PacketRouter::connect(IPacketEndpoint& packetEndpoint, uint8_t packetType)
{
	std::scoped_lock lock(endpointMapMutex);
    endpointMap[packetType] = &packetEndpoint;
}

void PacketRouter::send(std::vector<uint8_t> packet, uint8_t packetType)
{
	packet.push_back(packetType);
	packetTransceiver->sendPacket(std::move(packet));
}

void PacketRouter::routePacket(std::vector<uint8_t> packet)
{
    uint8_t packetType = packet.back();
    packet.pop_back();
    endpointMapMutex.lock();
    if (endpointMap[packetType] != nullptr)
    {
        endpointMap[packetType]->handlePacket(packet, packetType);
    }
    endpointMapMutex.unlock();
}