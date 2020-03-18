#include "PacketTransceiver.h"
#include <iostream>

PacketTransceiver::PacketTransceiver(boost::asio::ip::tcp::socket* socket)
{
    this->socket = socket;
    senderThread = std::thread(&PacketTransceiver::senderThreadFunction, this);
    receiverThread = std::thread(&PacketTransceiver::receiverThreadFunction, this);
}

void PacketTransceiver::sendPacket(std::vector<uchar> packet)
{
    senderMutex.lock();
	this->senderQueue.push(std::move(packet));
    senderMutex.unlock();
    senderCondition.notify_one();
}

std::vector<uchar> PacketTransceiver::receivePacket()
{
    std::scoped_lock lock(receiverMutex);
    std::vector<uchar> packet = std::move(receiverQueue.front());
    receiverQueue.pop();
	return std::move(packet);
}

bool PacketTransceiver::isPacketReady()
{
    std::scoped_lock lock(receiverMutex);
	return receiverQueue.size() > 0;
}

std::condition_variable* PacketTransceiver::getReceiveCondVar()
{
    return &receiveCondition;
}

void PacketTransceiver::senderThreadFunction()
{
    std::vector<uchar> sendBuf;
    std::mutex condVarMutex;//FIX this s h i t
    std::condition_variable condVar;
    while (true)
    {
        while (senderQueue.size() == 0)
        {
            std::unique_lock<std::mutex> lock(condVarMutex);
            senderCondition.wait(lock);//Note: U NEED TO OWN mutex!!!!! before calling wait 
            lock.unlock(); //and this
        }
        while (senderQueue.size() > 0)
        {
            senderMutex.lock();
            sendBuf = std::move(senderQueue.front());
            senderQueue.pop();
            senderMutex.unlock();
            size_t sendSize = sendBuf.size();
            boost::asio::write(*socket, boost::asio::buffer(&sendSize, sizeof(size_t)));
            boost::asio::write(*socket, boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
        }
    }
}

void PacketTransceiver::receiverThreadFunction()
{
    uchar* receiveBuf = new uchar[RECEVIER_BUF_SIZE];
    size_t bufReceiveSize = 0;
    size_t packetSize = 0;
    size_t receiveSize = 0;
    while (true)
    {
        try
        {
            receiveSize = socket->receive(boost::asio::buffer(receiveBuf + bufReceiveSize, RECEVIER_BUF_SIZE - bufReceiveSize));
            bufReceiveSize += receiveSize;
        }
        catch (boost::system::system_error e)
        {
            std::cout << "error" << std::endl;
        }
        packetSize = *(reinterpret_cast<size_t*>(receiveBuf));
        if (packetSize > RECEVIER_BUF_SIZE)
        {
            bufReceiveSize = 0;
        }
        size_t skipBytes = 0;
        while (bufReceiveSize > skipBytes + packetSize + sizeof(size_t))
        {
            receiverMutex.lock();
            receiverQueue.push(std::vector<uchar>(receiveBuf + skipBytes + sizeof(size_t), receiveBuf + skipBytes + sizeof(size_t) + packetSize));
            receiverMutex.unlock();
            receiveCondition.notify_one();
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
}
