#include "PacketRouter.h"
#include <algorithm>

PacketRouter::PacketRouter()
{
}

void PacketRouter::connectSource(PacketTransceiver& packetTransceiver)
{
    sourceMapMutex.lock();
    packetTransceiverList.push_back(&packetTransceiver);
    sourceMapMutex.unlock();
}

void PacketRouter::routePacket(const PacketTransceiver& packetTransceiver, std::vector<uint8_t> packet)
{
    sourceMapMutex.lock();
    for (auto it = packetTransceiverList.begin(); it != packetTransceiverList.end(); ++it)
    {
        if (*it != &packetTransceiver)
        {
            (*it)->sendPacket(packet);
        }
    }
    sourceMapMutex.unlock();
}