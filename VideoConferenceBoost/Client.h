#pragma once
#include <SFML/Graphics.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <vector>
#include <PacketTransceiver.h>
#include <ExceptionTransporter.h>
#include "IPacketEndpoint.h"
#include "MicrophoneRecorder.h"
#include "MicrophoneStream.h"
#include "PacketRouter.h"
#include "VideoRecorder.h"
#include "VideoStream.h"
#include "GUIButton.h"
#include "GUITextField.h"

class Client : public IPacketEndpoint
{
public:
	Client();
	Client(const Client&) = delete;
	Client(Client&&) = delete;
	Client& operator= (const Client&) = delete;
	Client& operator= (Client&&) = delete;
	~Client();
	void start();
	virtual void handlePacket(const std::vector<uint8_t> packet, uint8_t packetType);
private:
	constexpr static unsigned int WINDOW_WIDTH = 1280;
	constexpr static unsigned int WINDOW_HEIGHT = 720;
	constexpr static unsigned int MIN_WINDOW_WIDTH = 1280;
	constexpr static unsigned int MIN_WINDOW_HEIGHT = 720;
	constexpr static std::string_view WINDOW_TITLE = "VideoConference";
	constexpr static unsigned int MAXIMUM_WINDOW_FPS = 30;
	
	enum class State { Connected, NotConnected };
	State state = State::NotConnected;
	void setState(State state);

	enum class ServerStatus {ClientConnected, ClientDisconnected};
	ServerStatus serverStatus = ServerStatus::ClientDisconnected;

	void connect();
	void disconnect();
	void setStatus(const std::string& status, const sf::Color& color);
	void setLastError(const std::string& lastError);

	boost::asio::io_service ioservice;

	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE.data());
	void resizeWindow(unsigned int width, unsigned int height);
	sf::Font font;
	sf::Text statusText;
	sf::Text lastErrorText;
	sf::Clock myClock;
	sf::Time elapsedTime;

	PacketTransceiver packetTransceiver;
	PacketRouter packetRouter = PacketRouter(packetTransceiver);
	MicrophoneStream microphoneStream;
	VideoStream videoStream;
	VideoRecorder videoRecorder = VideoRecorder(packetRouter);
	MicrophoneRecorder* microphoneRecorder = nullptr;
	void sendSetup();

	GUIButton connectButton;
	GUIButton disconnectButton;
	GUITextField addressTextField;
	GUITextField portTextField;
};
