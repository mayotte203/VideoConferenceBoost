#include "Server.h"
#include <vector> 
#include <ExceptionTransporter.h>
#include <types.h>
#include <iostream>

Server::Server(unsigned short serverPort) :port(serverPort)
{
    for (size_t i = 0; i < CLIENT_MAX_COUNT; ++i)
    {
        packetTransceiverArr[i].connectRouter(packetRouter);
        packetRouter.connectSource(packetTransceiverArr[i]);
    }
}

Server::~Server()
{
}

void Server::start()
{
    acceptorThread = std::thread(&Server::acceptorFunction, this);
    std::mutex exceptionMutex;
    while (working)
    {
        std::unique_lock lock(exceptionMutex);
        while (ExceptionTransporter::isEmpty())
        {
            ExceptionTransporter::getReadyCondition()->wait(lock);
        }
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
                            if (j != i && packetTransceiverArr[j].isConnected())
                            {
                                std::vector<uint8_t> packet{ PacketType::ServerType::ClientDisconnected, PacketType::Server };
                                packetTransceiverArr[j].sendPacket(packet);
                            }
                        }
                        acceptorCondtion.notify_all();
                    }
                }
            }
            if (strcmp(excepetionPair.second.what(), "Acceptor start error") == 0)
            {
                std::cout << "Can't start server" << std::endl;
                stop();
            }
        }
        lock.unlock();
    }
}

void Server::stop()
{
    working = false;
    ioservice.stop();
    if (acceptorThread.joinable())
    {
        acceptorThread.join();
    }
}

void Server::acceptorFunction()
{
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port);
    boost::asio::ip::tcp::acceptor acceptor(ioservice, ep.protocol());
    boost::asio::ip::tcp::socket socket(ioservice);
    try
    {
        acceptor.bind(ep);
        acceptor.listen();
    }
    catch (boost::system::system_error exception)
    {
        ExceptionTransporter::transportException(this, std::exception("Acceptor start error"));
        return;
    }

    std::mutex acceptorMutex;
    while (working)
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
        lock.unlock();
    }
}
