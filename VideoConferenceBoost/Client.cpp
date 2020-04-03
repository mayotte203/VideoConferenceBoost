#include "Client.h"

Client::Client()
{
}

Client::~Client()
{
}

void Client::start()
{
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

    connectButton.setSize(sf::Vector2f(200, 50));
    connectButton.setPosition(sf::Vector2f(50, 650));
    connectButton.setCallbackFunction(std::bind(&Client::connect, this));
    connectButton.setString("Connect");

    disconnectButton.setSize(sf::Vector2f(200, 50));
    disconnectButton.setPosition(sf::Vector2f(300, 650));
    disconnectButton.setCallbackFunction(std::bind(&Client::disconnect, this));
    disconnectButton.setString("Disconnect");

    addressTextField.setPosition(sf::Vector2f(50, 580));
    addressTextField.setSize(sf::Vector2f(450, 50));
    addressTextField.setGhostString("Enter address");

    packetRouter.connect(microphoneStream, PacketType::Sound);
    packetRouter.connect(videoStream, PacketType::Image);
    packetRouter.connect(*this, PacketType::System);
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
                setStatus("Not connected", sf::Color::Red);
                setLastError("Connection aborted");
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
}

void Client::handlePacket(const std::vector<uint8_t> packet, uint8_t packetType)
{
}

void Client::connect()
{
    if (state == State::NotConnected)
    {
        try
        {
            std::string addressString = addressTextField.getInputString();
            std::string ipString = std::string(addressString, 0, addressString.find_first_of(":"));
            std::string portString = std::string(addressString, addressString.find_first_of(":") + 1);
            boost::asio::ip::address ip = boost::asio::ip::address::from_string(ipString);
            boost::asio::ip::tcp::socket socket(ioservice);
            boost::asio::ip::tcp::endpoint ep(ip, std::stoi(portString));
            socket.open(boost::asio::ip::tcp::v4());
            socket.connect(ep);
            packetTransceiver.connect(std::move(socket));
            state = State::Connected;
            setStatus("Connected", sf::Color::Green);
            setLastError("");
        }
        catch (boost::system::system_error exception)
        {
            setLastError("Connection error");
        }
        catch (std::exception exception)
        {
            setLastError("Invalid IP-address");
        }
    }
}

void Client::disconnect()
{
    if (state == State::Connected)
    {
        packetTransceiver.disconnect();
        state = State::NotConnected;
        setStatus("Not connected", sf::Color::Red);
    }
    setLastError("");
}

void Client::setStatus(const std::string& status,const sf::Color& color)
{
    statusText.setString(status);
    statusText.setFillColor(color);
}

void Client::setLastError(const std::string& lastError)
{
    lastErrorText.setString(lastError);
}
