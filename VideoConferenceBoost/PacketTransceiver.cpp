#include "PacketTransceiver.h"
#include <iostream>

PacketTransceiver::PacketTransceiver(boost::asio::io_service& ioservice) :socket(boost::asio::ip::tcp::socket(ioservice))
{
    this->ioservice = &ioservice;
}

void PacketTransceiver::sendPacket(std::vector<uchar> packet)
{
    if (sending)
    {
        senderQueueMutex.lock();
        this->senderQueue.push(std::move(packet));
        senderQueueMutex.unlock();
        senderQueueCondition.notify_one();
    }
}

bool PacketTransceiver::isReceiving()
{
    std::scoped_lock lock(receiverMutex);
    return receiving;
}

bool PacketTransceiver::isSending()
{
    std::scoped_lock lock(senderMutex);
    return sending;
}

void PacketTransceiver::setReceiving(bool isReceiving)
{
    std::scoped_lock lock(receiverMutex);
    receiving = isReceiving;
    if (receiving)
    {
        receiverCondition.notify_one();
    }
    else
    {

    }
}

void PacketTransceiver::setSending(bool isSending)
{
    std::scoped_lock lock(senderMutex);
    sending = isSending;
    if (sending)
    {
        senderCondition.notify_one();
    }
    else
    {
        senderQueueMutex.lock();
        senderQueue = std::queue<std::vector<uchar>>();
        senderQueueMutex.unlock();
    }
}

void PacketTransceiver::connectRouter(IPacketRouter& packetRouter)
{
    this->packetRouter = &packetRouter;
}

void PacketTransceiver::connect(boost::asio::ip::address address, int port)
{
    boost::asio::ip::tcp::endpoint ep(address, port);
    socket.open(boost::asio::ip::tcp::v4());
    try
    {
        socket.connect(ep);
    }
    catch (boost::system::system_error e)
    {
        std::cout << "error" << std::endl;
    }
    sending = true;
    receiving = true;
    senderThread = std::thread(&PacketTransceiver::senderThreadFunction, this);
    receiverThread = std::thread(&PacketTransceiver::receiverThreadFunction, this);
}

void PacketTransceiver::disconnect()
{
    sending = false;
    receiving = false;
    senderQueueCondition.notify_one();
    if (senderThread.joinable())
    {
        senderThread.join();
    }
    if (receiverThread.joinable())
    {
        receiverThread.join();
    }
    if (socket.is_open())
    {
        socket.close();
    }
}

void PacketTransceiver::senderThreadFunction()
{
    std::vector<uchar> sendBuf;
    std::unique_lock<std::mutex> senderQueueLock(senderQueueMutex, std::defer_lock);
    std::unique_lock<std::mutex> senderLock(senderMutex, std::defer_lock);
    while (sending)
    {
        senderQueueLock.lock();
        while (senderQueue.size() == 0 && sending)
        {
            senderQueueCondition.wait(senderQueueLock);//Note: U NEED TO OWN mutex!!!!! before calling wait 
        }    
        if (senderQueue.size() != 0)
        {
            sendBuf = std::move(senderQueue.front());
            senderQueue.pop();
        }
        senderQueueLock.unlock();
        if (sending)
        {
            size_t sendSize = sendBuf.size();
            try
            {
                boost::asio::write(socket, boost::asio::buffer(&sendSize, sizeof(size_t)));
                boost::asio::write(socket, boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
            }
            catch (boost::system::system_error exception)
            {
                std::cout << exception.what() << std::endl;
                ExceptionTransporter::throwException(ExceptionTransporter::Invoker::SenderThread, std::exception("Connection Aborted"));
            }
        }
    }
}

void PacketTransceiver::receiverThreadFunction()
{
    uchar* receiveBuf = new uchar[RECEVIER_BUF_SIZE];
    size_t bufReceiveSize = 0;
    size_t packetSize = 0;
    size_t receiveSize = 0;
    std::unique_lock<std::mutex> receiverLock(receiverMutex, std::defer_lock);
    while (receiving)
    {
        try
        {
            receiveSize = socket.receive(boost::asio::buffer(receiveBuf + bufReceiveSize, RECEVIER_BUF_SIZE - bufReceiveSize));
            bufReceiveSize += receiveSize;
        }
        catch (boost::system::system_error exception)
        {
            std::cout << exception.what() << std::endl;
            ExceptionTransporter::throwException(ExceptionTransporter::Invoker::ReceiverThread, std::exception("Connection Aborted"));
        }
        packetSize = *(reinterpret_cast<size_t*>(receiveBuf));
        if (packetSize > RECEVIER_BUF_SIZE)
        {
            bufReceiveSize = 0;
        }
        size_t skipBytes = 0;
        while (bufReceiveSize > skipBytes + packetSize + sizeof(size_t))
        {
            if(packetRouter != nullptr)
            packetRouter->routePacket(std::vector<uchar>(receiveBuf + skipBytes + sizeof(size_t), receiveBuf + skipBytes + sizeof(size_t) + packetSize));
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
