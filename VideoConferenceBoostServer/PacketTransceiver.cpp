#include "PacketTransceiver.h"
#include <iostream>

PacketTransceiver::PacketTransceiver()
{

}

PacketTransceiver::~PacketTransceiver()
{
    disconnect();
}

void PacketTransceiver::connect(boost::asio::ip::tcp::socket&& socket)
{
    this->socket = new boost::asio::ip::tcp::socket(std::move(socket));
    connected = true;
    senderThread = std::thread(&PacketTransceiver::senderThreadFunction, this);
    receiverThread = std::thread(&PacketTransceiver::receiverThreadFunction, this);
}

void PacketTransceiver::disconnect()
{
    connected = false;
    senderQueueCondition.notify_one();
    if (senderThread.joinable())
    {
        senderThread.join();
    }
    if (receiverThread.joinable())
    {
        receiverThread.join();
    }
    if (socket != nullptr)
    {
        delete socket;
    }
}

void PacketTransceiver::sendPacket(std::vector<uint8_t> packet)
{
    if (connected)
    {
        senderQueueMutex.lock();
        this->senderQueue.push(std::move(packet));
        senderQueueMutex.unlock();
        senderQueueCondition.notify_one();
    }
}

void PacketTransceiver::connectRouter(int ID, IPacketRouter& packetRouter)
{
    this->ID = ID;
    this->packetRouter = &packetRouter;
}

bool PacketTransceiver::isConnected()
{
    return connected;
}

void PacketTransceiver::senderThreadFunction()
{
    std::vector<uint8_t> sendBuf;
    std::unique_lock<std::mutex> senderQueueLock(senderQueueMutex, std::defer_lock);
    std::unique_lock<std::mutex> senderLock(senderMutex, std::defer_lock);
    while (connected)
    {
        senderQueueLock.lock();
        while (senderQueue.size() == 0 && connected)
        {
            senderQueueCondition.wait(senderQueueLock);
        }
        if (senderQueue.size() != 0)
        {
            sendBuf = std::move(senderQueue.front());
            senderQueue.pop();
        }
        senderQueueLock.unlock();
        if (connected)
        {
            size_t sendSize = sendBuf.size();
            try
            {
                boost::asio::write(*socket, boost::asio::buffer(&sendSize, sizeof(size_t)));
                boost::asio::write(*socket, boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
            }
            catch (boost::system::system_error exception)
            {
                connected = false;
                ExceptionTransporter::throwException(this, std::exception("Connection Aborted"));
            }
        }
    }
}

void PacketTransceiver::receiverThreadFunction()
{
    uint8_t* receiveBuf = new uint8_t[RECEVIER_BUF_SIZE];
    size_t bufReceiveSize = 0;
    size_t packetSize = 0;
    size_t receiveSize = 0;
    std::unique_lock<std::mutex> receiverLock(receiverMutex, std::defer_lock);
    while (connected)
    {
        try
        {
            receiveSize = socket->receive(boost::asio::buffer(receiveBuf + bufReceiveSize, RECEVIER_BUF_SIZE - bufReceiveSize));
            bufReceiveSize += receiveSize;
        }
        catch (boost::system::system_error exception)
        {
            connected = false;
            ExceptionTransporter::throwException(this, std::exception("Connection Aborted"));
        }
        packetSize = *(reinterpret_cast<size_t*>(receiveBuf));
        if (packetSize > RECEVIER_BUF_SIZE)
        {
            bufReceiveSize = 0;
        }
        size_t skipBytes = 0;
        while (bufReceiveSize > skipBytes + packetSize + sizeof(size_t))
        {
            if (packetRouter != nullptr)
                packetRouter->routePacket(ID, std::vector<uint8_t>(receiveBuf + skipBytes + sizeof(size_t), receiveBuf + skipBytes + sizeof(size_t) + packetSize));
            skipBytes += packetSize + sizeof(size_t);
            packetSize = *(reinterpret_cast<size_t*>(receiveBuf + skipBytes));
        }
        if (skipBytes > 0)
        {
            for (size_t i = 0; i < bufReceiveSize - skipBytes; ++i)
            {
                receiveBuf[i] = receiveBuf[i + skipBytes];
            }
            bufReceiveSize = bufReceiveSize - skipBytes;
        }
    }
    delete[] receiveBuf;
}
