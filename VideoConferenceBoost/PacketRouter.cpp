#include "PacketRouter.h"
#include <algorithm>

PacketRouter::PacketRouter(PacketTransceiver& packetTr)
{
	packetTransceiver = &packetTr;
    packetRouterThread = std::thread(&PacketRouter::packetRouterThreadFunction, this);
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
	packetTransceiver->sendPacket(packet);
}

void PacketRouter::packetRouterThreadFunction()
{
    std::condition_variable* condVar = packetTransceiver->getReceiveCondVar();
    std::unique_lock<std::mutex> lock(endpointMapMutex, std::defer_lock);
    while (true)//Think about threads loops
    {
        lock.lock();
        while (!packetTransceiver->isPacketReady())
        {
            condVar->wait(lock);
        }
        while (packetTransceiver->isPacketReady())
        {
            std::vector<uchar> packet = packetTransceiver->receivePacket();
            uchar packetType = packet.back();
            packet.pop_back();
            if (endpointMap[packetType] != nullptr)
            {
                endpointMap[packetType]->handlePacket(packet, packetType);
            }
        }
        lock.unlock();
    }
}
