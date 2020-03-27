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
    system("chcp 1251 > nul");
    boost::asio::io_service ioservice;

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "VideoConference");

    PacketTransceiver packetTransceiver(ioservice);
    PacketRouter packetRouter(packetTransceiver);
    MicrophoneStream microphoneStream;
    packetRouter.connect(microphoneStream, PacketType::Sound);
    VideoStream videoStream;
    packetRouter.connect(videoStream, PacketType::Image);
    VideoRecorder videoRecorder(packetRouter);
    MicrophoneRecorder microphoneRecorder(packetRouter);
    microphoneRecorder.start();
    packetTransceiver.connect(boost::asio::ip::address::from_string("192.168.0.102"), 50005);
    
    sf::Font font;
    sf::Text statusText;
    font.loadFromFile("3966.ttf");
    statusText.setFillColor(sf::Color::Green);
    statusText.setFont(font);
    statusText.setCharacterSize(40);
    statusText.setPosition(50, 520);
    statusText.setString("Connected");

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
                    packetTransceiver.disconnect();
                    state = State::NotConnected;
                    statusText.setString("Not Connected");
                    statusText.setFillColor(sf::Color::Red);
                }
                if (event.key.code == sf::Keyboard::W && state == State::NotConnected)
                {
                    packetTransceiver.connect(boost::asio::ip::address::from_string("192.168.0.102"), 50005);
                    state = State::Connected;
                    statusText.setString("Connected");
                    statusText.setFillColor(sf::Color::Green);
                }
            }
        }
        while (!ExceptionTransporter::isEmpty())
        {
            auto excepetionPair = ExceptionTransporter::retrieveException();
            if (strcmp(excepetionPair.second.what(), "Connection Aborted") == 0)
            {
                packetTransceiver.disconnect();
                state = State::NotConnected;
                statusText.setString("Not Connected");
                statusText.setFillColor(sf::Color::Red);
            }
        }
        elapsedTime += clock.restart();
        if (elapsedTime > sf::milliseconds(1000 / 30))
        {
            window.clear();
            window.draw(videoRecorder);
            window.draw(videoStream);
            window.draw(statusText);
            window.display();
            elapsedTime -= sf::milliseconds(1000 / 30);
        }
    }
    return 0;
}