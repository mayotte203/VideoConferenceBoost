#include <SFML/Graphics.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <chrono>
#include <ctime>
#include <cmath>
#include <random>
#include "types.h"
#include "PacketTransceiver.h"
#include "MicrophoneRecorder.h"
#include "MicrophoneStream.h"
#include "PacketRouter.h"
#include "VideoRecorder.h"
#include "VideoStream.h"
#include "ExceptionTransporter.h"

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int  WINDOW_HEIGHT = 720;

int main()
{
    boost::asio::io_service ioservice;
    boost::asio::ip::tcp::socket socket(ioservice);
    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("192.168.0.102"), 50005);
    socket.open(boost::asio::ip::tcp::v4());
    try
    {
        socket.connect(ep);
    }
    catch (boost::system::system_error e)
    {
        std::cout << "error" << std::endl;
    }

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "VideoConference");

    PacketTransceiver packetTransceiver(socket);
    PacketRouter packetRouter(packetTransceiver);
    MicrophoneStream microphoneStream;
    packetRouter.connect(microphoneStream, PacketType::Sound);
    VideoStream videoStream;
    packetRouter.connect(videoStream, PacketType::Image);
    VideoRecorder videoRecorder(packetRouter);
    MicrophoneRecorder microphoneRecorder(packetRouter);
    microphoneRecorder.start();
    packetTransceiver.setSending(true);
    packetTransceiver.setReceiving(true);
    sf::Clock clock;
    sf::Time elapsedTime;
    clock.restart();
    enum class State{Connected, NotConnected};
    State state = State::Connected;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Q && state == State::Connected)
                {
                    packetTransceiver.setReceiving(false);
                    packetTransceiver.setSending(false);
                    socket.close();
                    state = State::NotConnected;
                }
                if (event.key.code == sf::Keyboard::W && state == State::NotConnected)
                {
                    socket.open(boost::asio::ip::tcp::v4());
                    socket.connect(ep);
                    packetTransceiver.setSending(true);
                    packetTransceiver.setReceiving(true);
                    state = State::Connected;
                }
            }
        }
        while (!ExceptionTransporter::isEmpty())
        {
            auto excepetionPair = ExceptionTransporter::retrieveException();
        }
        elapsedTime += clock.restart();
        if (elapsedTime > sf::milliseconds(1000 / 30))
        {
            window.clear();
            window.draw(videoRecorder);
            window.draw(videoStream);
            window.display();
            elapsedTime -= sf::milliseconds(1000 / 30);
        }
    }
    return 0;
}