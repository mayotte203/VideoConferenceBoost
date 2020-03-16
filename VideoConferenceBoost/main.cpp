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

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int  WINDOW_HEIGHT = 720;
constexpr unsigned int CAMERA_WIDTH = 640;
constexpr unsigned int CAMERA_HEIGHT = 480;

#define SINGLE_PC 

//
void receiverThreadFunction(boost::asio::ip::tcp::socket* socket, std::mutex* socketMutex, std::vector<uchar>* packetBuf, std::mutex* packetBufMutex, bool* packetReady)
{
    uchar* receiveBuf = new uchar[500000];
    size_t bufReceiveSize = 0;
    size_t packetSize = 0;
    size_t receiveSize = 0;
    while (true)
    {
        try
        {
            receiveSize = socket->receive(boost::asio::buffer(receiveBuf + bufReceiveSize, 500000 - bufReceiveSize));
            bufReceiveSize += receiveSize;
        }
        catch (boost::system::system_error e)
        {
            std::cout << "error" << std::endl;
        }
        packetSize = *(reinterpret_cast<size_t*>(receiveBuf));
        if (packetSize > 500000)
        {
            bufReceiveSize = 0;
        }
        size_t skipBytes = 0;
        packetBufMutex->lock();
        while (bufReceiveSize > packetSize + sizeof(size_t))
        {
            //packetBufMutex->lock();
            *packetBuf = std::vector<uchar>(receiveBuf + sizeof(size_t), receiveBuf + sizeof(size_t) + packetSize);
            *packetReady = true;
            //packetBufMutex->unlock();
            for (size_t i = 0; i < bufReceiveSize - packetSize - sizeof(size_t); ++i)
            {
                receiveBuf[i] = receiveBuf[i + packetSize + sizeof(size_t)];
            }
            bufReceiveSize = bufReceiveSize - packetSize - sizeof(size_t);
            packetSize = *(reinterpret_cast<size_t*>(receiveBuf));
        }
        packetBufMutex->unlock();
    }
}

void senderThreadFunction(boost::asio::ip::tcp::socket* socket, std::mutex* socketMutex, std::vector<uchar>* packetBuf, std::mutex* packetBufMutex, bool* packetReady)
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
            size_t sendSize = sendBuf.size();
            boost::asio::write(*socket, boost::asio::buffer(&sendSize, sizeof(size_t)));
            boost::asio::write(*socket, boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
            //socket->send(boost::asio::buffer(&sendSize, sizeof(size_t)));
            //socket->send(boost::asio::buffer(reinterpret_cast<void*>(sendBuf.data()), sendBuf.size()));
        }
        else
        {
            packetBufMutex->unlock();
        }
    }
}

void imagePacketFunction(sf::Texture *texture, PacketTransceiver *packetTr, std::mutex *textureMutex, std::condition_variable *condVar, MicrophoneStream *micStream)
{
    sf::Image image;
    sf::SoundBuffer buffer;
    sf::Sound sound(buffer);
    while (true)
    {
        std::unique_lock<std::mutex> lock(*textureMutex); 
        while (!packetTr->isPacketReady())
        {
            condVar->wait(lock);
        }
        lock.unlock();
        while (packetTr->isPacketReady())
        {
            std::vector<uchar> imagePacket = packetTr->receivePacket();
            switch (imagePacket.back())
            {
                case PacketType::Image:
                {
                    cv::Mat frameRGB = cv::imdecode(imagePacket, -1);
                    cv::Mat frameRGBA;
                    cv::cvtColor(frameRGB, frameRGBA, cv::COLOR_BGR2RGBA);
                    image.create(frameRGBA.cols, frameRGBA.rows, frameRGBA.ptr());
                    lock.lock();
                    texture->update(image);
                    lock.unlock();
                    break;
                }
                case PacketType::Sound:
                {
                    imagePacket.pop_back();
                    /*sound.stop();
                    buffer.loadFromSamples(reinterpret_cast<sf::Int16*>(imagePacket.data()), imagePacket.size() / 2, 1, 44100);
                    sound.play();*/
                    micStream->addSamples(imagePacket);
                    if (micStream->getStatus() != sf::SoundSource::Playing)
                    {
                        micStream->stop();
                        micStream->play();
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
            
        }
    }
}

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
#ifndef SINGLE_PC
    cv::VideoCapture WebcamVC(0);
#endif // !SINGLE_PC
    cv::Mat frameRGB, frameRGBA, WebcamMat;
    //Local camera
    sf::Image image;
    sf::Texture texture;
    sf::Sprite sprite;
    sf::RenderTexture renderTexture;
    sf::Font font;
    sf::Text text;
    image.create(CAMERA_WIDTH, CAMERA_HEIGHT);
    texture.loadFromImage(image);
    sprite.setTexture(texture);
    renderTexture.create(CAMERA_WIDTH, CAMERA_HEIGHT);
    font.loadFromFile("3966.ttf");
    text.setFillColor(sf::Color::Black);
    text.setFont(font);
    text.setCharacterSize(30);
    text.setPosition(50, 200);
    //Remote camera
    sf::Image image1;
    sf::Texture texture1;
    sf::Sprite sprite1;
    image1.create(CAMERA_WIDTH, CAMERA_HEIGHT);
    texture1.loadFromImage(image1);
    sprite1.setTexture(texture1);
    sprite1.setPosition(sf::Vector2f(CAMERA_WIDTH, 0));

    std::vector<uchar> receiveBuf;
    std::vector<uchar> sendBuf;
    //std::mutex socketMutex, receiveMutex, sendMutex;
    //bool sendReady = false;
    //bool receiveReady = false;
    //std::thread senderThread(senderThreadFunction, &socket, &socketMutex, &sendBuf, &sendMutex, &sendReady);
    //std::thread receiverThread(receiverThreadFunction, &socket, &socketMutex, &receiveBuf, &receiveMutex, &receiveReady);

    std::condition_variable condVar;
    PacketTransceiver packetTr(&socket, &condVar);
    std::mutex textureMutex;
    MicrophoneStream micStream;
    std::thread videoThread(imagePacketFunction, &texture1, &packetTr, &textureMutex, &condVar, &micStream);
    MicrophoneRecorder micRecorder(&packetTr);
    micRecorder.start();
    std::cout << micRecorder.getSampleRate() << std::endl;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
#ifdef SINGLE_PC
        std::stringstream ss;
        ss << time(0) << " " << rand() << std::endl;
        text.setString(ss.str());
        renderTexture.clear(sf::Color::White);
        renderTexture.draw(text);
        renderTexture.display();
        image = renderTexture.getTexture().copyToImage();
#else
        WebcamVC >> WebcamMat;
        cv::cvtColor(WebcamMat, frameRGBA, cv::COLOR_BGR2RGBA);
        image.create(frameRGBA.cols, frameRGBA.rows, frameRGBA.ptr());
#endif // SINGLE_PC

        texture.update(image);
        window.clear();
        window.draw(sprite);
        textureMutex.lock();
        window.draw(sprite1);
        textureMutex.unlock();
        window.display();

#ifdef SINGLE_PC
        uchar* pixels = reinterpret_cast<uchar*>(const_cast<sf::Uint8*>(image.getPixelsPtr()));
        cv::Mat imageMat(cv::Size(image.getSize().x, image.getSize().y), CV_8UC4, pixels);
        cv::cvtColor(imageMat, frameRGB, cv::COLOR_RGBA2BGR);
        cv::imencode(".jpg", frameRGB, sendBuf);
#else
        cv::imencode(".jpg", WebcamMat, sendBuf);
#endif //SINGLE_PC

        sendBuf.push_back(PacketType::Image);
        packetTr.sendPacket(sendBuf);
    }

    return 0;
}