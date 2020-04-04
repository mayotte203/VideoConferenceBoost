#include "Client.h"
#include "types.h"

Client::Client()
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
    addressTextField.setSize(sf::Vector2f(300, 50));
    addressTextField.setGhostString("IP");

    portTextField.setPosition(sf::Vector2f(370, 580));
    portTextField.setSize(sf::Vector2f(130, 50));
    portTextField.setGhostString("Port");

    videoStream.setPosition(640, 0);

    packetRouter.connect(microphoneStream, PacketType::Sound);
    packetRouter.connect(videoStream, PacketType::Image);
    packetRouter.connect(*this, PacketType::Server);
    packetRouter.connect(*this, PacketType::Setup);
    if (sf::SoundRecorder::isAvailable())
    {
        microphoneRecorder = new MicrophoneRecorder(packetRouter);
    }
    else
    {
        MessageBox(
            NULL,
            (LPCWSTR)L"No audio input device found",
            (LPCWSTR)L"Audio will not be recorded",
            MB_ICONWARNING | IDOK
        );
    }   
}

Client::~Client()
{
    if (microphoneRecorder != nullptr)
    {
        delete microphoneRecorder;
    }
    if (packetTransceiver.isConnected())
    {
        packetTransceiver.disconnect();
    }
}

void Client::start()
{
    if (microphoneRecorder != nullptr)
    {
        microphoneRecorder->start();
    }

    myClock.restart();
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
            {
                window.close();
                break;
            }
            case sf::Event::Resized:
            {
                resizeWindow(event.size.width, event.size.height);
                break;
            }
            }
            try
            {
                connectButton.updateEvent(event);
            }
            catch (std::exception exception)
            {
                setLastError(exception.what());
            }
            disconnectButton.updateEvent(event);
            addressTextField.updateEvent(event);
            portTextField.updateEvent(event);
        }
        while (!ExceptionTransporter::isEmpty())
        {
            auto excepetionPair = ExceptionTransporter::retrieveException();
            if (strcmp(excepetionPair.second.what(), "Connection Aborted") == 0)
            {
                packetTransceiver.disconnect();
                setState(State::NotConnected);
                setLastError("Connection aborted");
            }
        }
        elapsedTime += myClock.restart();
        if (elapsedTime > sf::milliseconds(1000 / 30))
        {
            window.clear(sf::Color::White);
            switch (state)
            {
            case State::Connected:
            {
                if (serverStatus == ServerStatus::ClientConnected)
                {
                    window.draw(videoStream);
                }
                break;
            }
            case State::NotConnected:
            {
                break;
            }
            }
            window.draw(videoRecorder);
            window.draw(connectButton);
            window.draw(disconnectButton);
            window.draw(statusText);
            window.draw(addressTextField);
            window.draw(portTextField);
            window.draw(lastErrorText);
            window.display();
            elapsedTime -= sf::milliseconds(1000 / 30);
        }
    }
}

void Client::handlePacket(const std::vector<uint8_t> packet, uint8_t packetType)
{
    switch (packetType)
    {
    case PacketType::Server:
    {
        switch (packet.back())
        {
        case PacketType::ServerType::ClientConnected:
        {
            serverStatus = ServerStatus::ClientConnected;
            sendSetup();
            break;
        }
        case PacketType::ServerType::ClientDisconnected:
        {
            serverStatus = ServerStatus::ClientDisconnected;
            break;
        }
        }
        break;
    }
    case PacketType::Setup:
    {
        switch (packet.back())
        {
        case PacketType::SetupType::SampleRate:
        {
            microphoneStream.setSampleRate(*(reinterpret_cast<const unsigned int*>(packet.data())));
            break;
        }
        }
        break;
    }
    }
}

void Client::setState(State state)
{
    switch (state)
    {
    case State::Connected:
    {
        setStatus("Connected", sf::Color::Green);
        break;
    }
    case State::NotConnected:
    {
        setStatus("Not connected", sf::Color::Red);
        break;
    }
    }
    this->state = state;
}

void Client::connect()
{
    if (state == State::NotConnected)
    {
        std::string ipString = addressTextField.getInputString();
        std::string portString = portTextField.getInputString();
        boost::asio::ip::address ip;
        unsigned int port;
        if (portString.size() > 5 || !std::all_of(portString.begin(), portString.end(), ::isdigit))
        {
            throw(std::exception("Invalid Port"));
        }
        try
        {
            ip = boost::asio::ip::address::from_string(ipString);
            port = std::stoi(portString);
        }
        catch (boost::system::system_error exception)
        {
            throw(std::exception("Invalid IP"));
        }
        catch (std::exception exception)
        {
            throw(std::exception("Invalid IP"));
        }
        try
        {
            boost::asio::ip::tcp::socket socket(ioservice);
            boost::asio::ip::tcp::endpoint ep(ip, port);
            socket.open(boost::asio::ip::tcp::v4());
            socket.connect(ep);
            packetTransceiver.connect(std::move(socket));
        }
        catch (boost::system::system_error exception)
        {
            throw(std::exception("Connection Error"));
        }
        setState(State::Connected);
        setLastError("");
    }
}

void Client::disconnect()
{
    if (state == State::Connected)
    {
        packetTransceiver.disconnect();
        setState(State::NotConnected);
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

void Client::resizeWindow(unsigned int width, unsigned int height)
{
    width = width > MIN_WINDOW_WIDTH ? width : MIN_WINDOW_WIDTH;
    height = height > MIN_WINDOW_HEIGHT ? height : MIN_WINDOW_HEIGHT;
    window.setSize(sf::Vector2u(width, height));
    window.setView(sf::View(sf::Vector2f(width / 2.0f, height / 2.0f), sf::Vector2f(width, height)));

    float videoStreamScale = std::min((width * 0.5f) / static_cast<float> (videoStream.getImageSize().x),
                                        (height * 0.7f) / static_cast<float> (videoStream.getImageSize().y));
    videoStream.setScale(videoStreamScale, videoStreamScale);
    videoStream.setPosition(width * 0.5f + (width * 0.5f - (videoStream.getImageSize().x * videoStreamScale)) * 0.5f, 0);
    float videoRecorderScale = std::min((width * 0.5f) / static_cast<float> (videoRecorder.getImageSize().x),
        (height * 0.7f) / static_cast<float> (videoRecorder.getImageSize().y));
    videoRecorder.setScale(videoRecorderScale, videoRecorderScale);
    videoRecorder.setPosition((width * 0.5f - (videoRecorder.getImageSize().x * videoStreamScale)) * 0.5f, 0);

    connectButton.setSize(sf::Vector2f(200.0f / 1280.0f * width, 50.0f / 720.0f * height));
    connectButton.setPosition(sf::Vector2f(50.0f / 1280.0f * width, 650.0f / 720.0f * height));

    disconnectButton.setSize(sf::Vector2f(250.0f / 1280.0f * width, 50.0f / 720.0f * height));
    disconnectButton.setPosition(sf::Vector2f(300.0f / 1280.0f * width, 650.0f / 720.0f * height));

    addressTextField.setSize(sf::Vector2f(300.0f / 1280.0f * width, 50.0f / 720.0f * height));
    addressTextField.setPosition(sf::Vector2f(50.0f / 1280.0f * width, 580.0f / 720.0f * height));

    portTextField.setSize(sf::Vector2f(180.0f / 1280.0f * width, 50.0f / 720.0f * height));
    portTextField.setPosition(sf::Vector2f(370.0f / 1280.0f * width, 580.0f / 720.0f * height));

    statusText.setCharacterSize(40.0f / 720.0f * height);
    statusText.setPosition(50.0f / 1280.0f * width, 520.0f / 720.0f * height);

    lastErrorText.setCharacterSize(20.0f / 720.0f * height);
    lastErrorText.setPosition(800.0f / 1280.0f * width, 690.0f / 720.0f * height);
}

void Client::sendSetup()
{
    if (microphoneRecorder != nullptr)
    {
        std::vector<uint8_t> packet;
        unsigned int sampleRate = microphoneRecorder->getSampleRate();
        for (size_t i = 0; i < sizeof(unsigned int); ++i)
        {
            packet.push_back(reinterpret_cast<uint8_t*>(&sampleRate)[i]);
        }
        packet.push_back(PacketType::SetupType::SampleRate);
        packet.push_back(PacketType::Setup);
        packetTransceiver.sendPacket(packet);
    }
}
