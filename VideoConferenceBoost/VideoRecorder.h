#pragma once
#include <opencv2/opencv.hpp>
#include <sfml/Graphics.hpp>
#include <thread>
#include <mutex>
#include "PacketRouter.h"

//#define SINGLE_PC

class VideoRecorder : public sf::Drawable, public sf::Transformable
{
public:
	VideoRecorder() = delete;
	VideoRecorder(PacketRouter& packetRouter);
	~VideoRecorder();
	sf::Vector2u getImageSize();
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	PacketRouter* packetRouter;
	cv::VideoCapture webcamVC;
	sf::Image videoImage;
	sf::Texture videoTexture;
	sf::Sprite videoSprite;
	mutable std::mutex videoTextureMutex;
	bool recording = true;
	std::thread videoRecorderThread;
	void videoRecorderThreadFunction();
};

