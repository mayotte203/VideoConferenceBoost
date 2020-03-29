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
#include "GUIButton.h"
#include "GUITextField.h"

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int  WINDOW_HEIGHT = 720;

enum class State { Connected, NotConnected };
State state = State::NotConnected;

boost::asio::io_service ioservice;

sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "VideoConference");

PacketTransceiver packetTransceiver(ioservice);
PacketRouter packetRouter(packetTransceiver);
MicrophoneStream microphoneStream;
VideoStream videoStream;
VideoRecorder videoRecorder(packetRouter);
//packetTransceiver.connect(boost::asio::ip::address::from_string("192.168.0.102"), 50005);


sf::Font font;
sf::Text statusText;
sf::Text lastErrorText;

sf::Clock myClock;
sf::Time elapsedTime;

GUIButton connectButton;
GUIButton disconnectButton;
GUITextField addressTextField;

void connect()
{
    if (state == State::NotConnected)
    {
        try
        {
            std::string addressString = addressTextField.getInputString();
            std::string ipString = std::string(addressString, 0, addressString.find_first_of(":"));
            std::string portString = std::string(addressString, addressString.find_first_of(":") + 1);
            boost::asio::ip::address ip = boost::asio::ip::address::from_string(ipString);
            packetTransceiver.connect(ip, std::stoi(portString));
            state = State::Connected;
            statusText.setString("Connected");
            statusText.setFillColor(sf::Color::Green);
            lastErrorText.setString("");
        }
        catch (boost::system::system_error exception)
        {
            lastErrorText.setString("Connection error");
        }
        catch (std::exception exception)
        {
            lastErrorText.setString("Invalid IP-address");
        }
    }
}

void disconnect()
{
    if (state == State::Connected)
    {
        packetTransceiver.disconnect();
        state = State::NotConnected;
        statusText.setString("Not connected");
        statusText.setFillColor(sf::Color::Red);
    }
    lastErrorText.setString("");
}

int main()
{
    system("chcp 1251 > nul");
    
    
    font.loadFromFile("3966.ttf");
    statusText.setFillColor(sf::Color::Red);
    statusText.setFont(font);
    statusText.setCharacterSize(40);
    statusText.setPosition(50, 520);
    statusText.setString("Not Connected");

    lastErrorText.setFillColor(sf::Color::Black);
    lastErrorText.setFont(font);
    lastErrorText.setCharacterSize(20);
    lastErrorText.setPosition(800, 690);
    lastErrorText.setString("");

    connectButton.setSize(sf::Vector2f(200,50));
    connectButton.setPosition(sf::Vector2f(50,650));
    connectButton.setCallbackFunction(connect);
    connectButton.setString("Connect");

    disconnectButton.setSize(sf::Vector2f(200, 50));
    disconnectButton.setPosition(sf::Vector2f(300, 650));
    disconnectButton.setCallbackFunction(disconnect);
    disconnectButton.setString("Disconnect");

    addressTextField.setPosition(sf::Vector2f(50, 580));
    addressTextField.setSize(sf::Vector2f(450, 50));
    addressTextField.setGhostString("Enter address");

    packetRouter.connect(microphoneStream, PacketType::Sound);
    packetRouter.connect(videoStream, PacketType::Image);
    MicrophoneRecorder microphoneRecorder(packetRouter);
    microphoneRecorder.start();

    myClock.restart();
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            connectButton.updateEvent(event);
            disconnectButton.updateEvent(event);
            addressTextField.updateEvent(event);
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
                lastErrorText.setString("Connection aborted");
            }
        }
        elapsedTime += myClock.restart();
        if (elapsedTime > sf::milliseconds(1000 / 30))
        {
            window.clear(sf::Color::White);
            window.draw(videoRecorder);
            if (state == State::Connected)
            {
                window.draw(videoStream);
            }
            window.draw(connectButton);
            window.draw(disconnectButton);
            window.draw(statusText);
            window.draw(addressTextField);
            window.draw(lastErrorText);
            window.display();
            elapsedTime -= sf::milliseconds(1000 / 30);
        }
    }
    return 0;
}