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

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int  WINDOW_HEIGHT = 720;
constexpr unsigned int CAMERA_WIDTH = 640;
constexpr unsigned int CAMERA_HEIGHT = 480;

#define SINGLE_PC 

void imagePacketFunction(sf::Texture *texture, PacketTransceiver *packetTr, std::mutex *textureMutex, MicrophoneStream *micStream)
{
    sf::Image image;
    std::condition_variable* condVar = packetTr->getReceiveCondVar();
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
                    micStream->addSamples(std::move(imagePacket));
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

    std::condition_variable condVar;
    PacketTransceiver packetTr(&socket);
    std::mutex textureMutex;
    PacketRouter packetRouter(packetTr);
    MicrophoneStream micStream;
    packetRouter.connect(micStream, PacketType::Sound);
    //std::thread videoThread(imagePacketFunction, &texture1, &packetTr, &textureMutex, &micStream);
    MicrophoneRecorder micRecorder(&packetTr);
    micRecorder.start();
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
        WebcamVC.read(WebcamMat);
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
        cv::imencode(".jpg", WebcamMat, sendBuf, std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 70});
#endif //SINGLE_PC

        sendBuf.push_back(PacketType::Image);
        packetTr.sendPacket(std::move(sendBuf));
    }

    return 0;
}