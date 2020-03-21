#include "PacketTransceiver.h"
#include <iostream>

PacketTransceiver::PacketTransceiver(boost::asio::ip::tcp::socket& socket)
{
    this->socket = &socket;
    senderThread = std::thread(&PacketTransceiver::senderThreadFunction, this);
    receiverThread = std::thread(&PacketTransceiver::receiverThreadFunction, this);
}

void PacketTransceiver::sendPacket(std::vector<uchar> packet)
{
    if (sending)
    {
        senderMutex.lock();
        this->senderQueue.push(std::move(packet));
        senderMutex.unlock();
        sendCondition.notify_one();
    }
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

bool PacketTransceiver::isReceiving()
{
    return receiving;
}

bool PacketTransceiver::isSending()
{
    return sending;
}

void PacketTransceiver::setReceiving(bool isReceiving)
{
    receiving = isReceiving;
}

void PacketTransceiver::setSending(bool isSending)
{
    sending = isSending;
}

void PacketTransceiver::senderThreadFunction()
{
    std::vector<uchar> sendBuf;
    std::unique_lock<std::mutex> lock(senderMutex, std::defer_lock);
    while (true)
    {
        lock.lock();
        while (senderQueue.size() == 0)
        {
            sendCondition.wait(lock);//Note: U NEED TO OWN mutex!!!!! before calling wait 
        }    
        sendBuf = std::move(senderQueue.front());
        senderQueue.pop();
        lock.unlock();
        size_t sendSize = sendBuf.size();
        boost::asio::write(*socket, boost::asio::buffer(&sendSize, sizeof(size_t)));
        boost::asio::write(*socket, boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
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
