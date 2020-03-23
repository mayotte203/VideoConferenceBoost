#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector>

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int  WINDOW_HEIGHT = 720;

void receiverThreadFunction(boost::asio::ip::tcp::socket* socket0, boost::asio::ip::tcp::socket* socket1, std::vector<uchar>* packetBuf, std::mutex* packetBufMutex, bool* packetReady)
{
    uchar* receiveBuf = new uchar[20000];
    size_t receiveSize = 0;
    while (true)
    {
        try
        {
            //receiveSize = socket0->receive(boost::asio::buffer(receiveBuf, 200000));
            receiveSize = boost::asio::read(*socket0, boost::asio::buffer(receiveBuf, 20000));
        }
        catch (boost::system::system_error e)
        {
            std::cout << e.what() << std::endl;
        }
        //socket1->send(boost::asio::buffer(receiveBuf, receiveSize));
        boost::asio::write(*socket1, boost::asio::buffer(receiveBuf, receiveSize));
        /*packetBufMutex->lock();
        *packetBuf = std::vector<uchar>(receiveBuf, receiveBuf + receiveSize);
        *packetReady = true;
        packetBufMutex->unlock();*/
    }
}

void senderThreadFunction(boost::asio::ip::tcp::socket* socket, std::vector<uchar>* packetBuf, std::mutex* packetBufMutex, bool* packetReady)
{
    std::vector<uchar> sendBuf;
    bool bufReady = false;
    while (true)
    {
        packetBufMutex->lock();
        if (*packetReady)
        {
            sendBuf = *packetBuf;
            *packetReady = false;
            packetBufMutex->unlock();
            socket->send(boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
        }
        else
        {
            packetBufMutex->unlock();
        }
    }
}

int main()
{
    system("chcp 1251 > nul");
    std::vector<uchar> buf[2];
    std::mutex bufMutex[2];
    bool bufReady[2] = { false, false };

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), 50005);
    boost::asio::io_service ioservice;
    boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
    acceptor.bind(ep);
    acceptor.listen();
    boost::asio::ip::tcp::socket socket0(ioservice);
    acceptor.accept(socket0);

    boost::asio::ip::tcp::socket socket1(ioservice);
    acceptor.accept(socket1);
    std::thread receiverThread1(receiverThreadFunction, &socket1, &socket0, &buf[1], &bufMutex[1], &bufReady[1]);
    std::thread receiverThread0(receiverThreadFunction, &socket0, &socket1, &buf[0], &bufMutex[0], &bufReady[0]);

    //std::thread senderThread0(senderThreadFunction, &socket0, &buf[1], &bufMutex[1], &bufReady[1]);
    //std::thread senderThread1(senderThreadFunction, &socket1, &buf[0], &bufMutex[0], &bufReady[0]);

    receiverThread0.join();
    receiverThread1.join();
    /*senderThread0.join();
    senderThread1.join();*/
    return 0;
}