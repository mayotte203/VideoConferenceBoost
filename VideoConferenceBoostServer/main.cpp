#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <vector> 
#include "PacketTransceiver.h"
#include "PacketRouter.h"
#include "ExceptionTransporter.h"
#include "types.h"

constexpr size_t CLIENT_MAX_COUNT = 2;

PacketTransceiver packetTransceiverArr[CLIENT_MAX_COUNT];
std::condition_variable acceptorCondtion;

void acceptorFunction()
{
    boost::asio::io_service ioservice;
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), 50005);
    boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
    boost::asio::ip::tcp::socket socket(ioservice);
    acceptor.bind(ep);
    acceptor.listen();
    std::mutex acceptorMutex;
    while (true)
    {
        for (size_t i = 0; i < CLIENT_MAX_COUNT; ++i)
        {
            if (!packetTransceiverArr[i].isConnected())
            {
                acceptor.accept(socket);
                packetTransceiverArr[i].connect(std::move(socket));
                for (size_t j = 0; j < CLIENT_MAX_COUNT; ++j)
                {
                    if (j != i && packetTransceiverArr[j].isConnected())
                    {
                        std::vector<uint8_t> packet{ PacketType::ServerType::ClientConnected, PacketType::Server };
                        packetTransceiverArr[i].sendPacket(packet);
                        packetTransceiverArr[j].sendPacket(packet);
                    }
                }
            }
        }
        std::unique_lock lock(acceptorMutex);
        acceptorCondtion.wait(lock);
        lock.release();
    }
}

int main()
{
    system("chcp 1251 > nul");
    PacketRouter packetRouter;
    for (size_t i = 0; i < CLIENT_MAX_COUNT; ++i)
    {
        packetTransceiverArr[i].connectRouter(packetRouter);
        packetRouter.connectSource(packetTransceiverArr[i]);
    }
    std::thread acceptorThread(acceptorFunction);
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
                        for (size_t j = 0; j < CLIENT_MAX_COUNT; ++j)
                        {
                            if (packetTransceiverArr[j].isConnected())
                            {
                                std::vector<uint8_t> packet{PacketType::ServerType::ClientDisconnected, PacketType::Server};
                                packetTransceiverArr[j].sendPacket(packet);
                            }
                        }
                        acceptorCondtion.notify_all();
                    }
                }
            }
        }
    }

    return 0;
}