#include "PacketRouter.h"
#include <algorithm>

PacketRouter::PacketRouter()
{
}

int PacketRouter::connectSource(PacketTransceiver& packetTransceiver)
{
    sourceMapMutex.lock();
    sourceMap[nextID] = &packetTransceiver;
    sourceMapMutex.unlock();
    return nextID++;
}

void PacketRouter::send(int sourceID, std::vector<uint8_t> packet, uint8_t packetType)
{
    packet.push_back(packetType);
    sourceMapMutex.lock();
    sourceMap.at(sourceID)->sendPacket(std::move(packet));
    sourceMapMutex.unlock();
}

void PacketRouter::routePacket(int sourceID, std::vector<uint8_t> packet)
{
    sourceMapMutex.lock();
    for (auto it = sourceMap.begin(); it != sourceMap.end(); ++it)
    {
        if (it->first != sourceID)
        {
            it->second->sendPacket(packet);
        }
    }
    sourceMapMutex.unlock();
}

int PacketRouter::nextID = 0;