#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <vector> 
#include "PacketTransceiver.h"
#include "PacketRouter.h"
#include "ExceptionTransporter.h"

constexpr size_t CLIENT_MAX_COUNT = 2;

int main()
{
    system("chcp 1251 > nul");
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), 50005);
    boost::asio::io_service ioservice;
    boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
    boost::asio::ip::tcp::socket socket(ioservice);
    acceptor.bind(ep);
    acceptor.listen();
    PacketTransceiver packetTransceiverArr[CLIENT_MAX_COUNT];
    PacketRouter packetRouter;
    for (size_t i = 0; i < CLIENT_MAX_COUNT; ++i)
    {
        packetTransceiverArr[i].connectRouter(packetRouter);
        packetRouter.connectSource(packetTransceiverArr[i]);
    }
    while (true)
    {
        while (!ExceptionTransporter::isEmpty())
        {
            auto excepetionPair = ExceptionTransporter::retrieveException();
            if (strcmp(excepetionPair.second.what(), "Connection Aborted") == 0)
            {
                for (int i = 0; i < CLIENT_MAX_COUNT; ++i)
                {
                    if (excepetionPair.first == &packetTransceiverArr[i])
                    {
                        packetTransceiverArr[i].disconnect();
                    }
                }
            }
        }
        for (size_t i = 0; i < CLIENT_MAX_COUNT; ++i)
        {
            if (!packetTransceiverArr[i].isConnected())
            {
                acceptor.accept(socket);
                packetTransceiverArr[i].connect(std::move(socket));
            }
        }
    }

    return 0;
}