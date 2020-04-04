#include "VideoStream.h"
#include <opencv2/opencv.hpp>

VideoStream::VideoStream()
{
	videoImage.create(640, 480);//TODO Redo initialization
	videoTexture.loadFromImage(videoImage);
	videoTexture.setSmooth(true);
	videoSprite.setTexture(videoTexture);
}

void VideoStream::handlePacket(const std::vector<uint8_t> packet, uint8_t packetType)
{
	cv::Mat frameRGB = cv::imdecode(packet, -1);
	cv::Mat frameRGBA;
	cv::cvtColor(frameRGB, frameRGBA, cv::COLOR_BGR2RGBA);
	videoImage.create(frameRGBA.cols, frameRGBA.rows, frameRGBA.ptr());
	videoTextureMutex.lock();
	videoTexture.loadFromImage(videoImage);
	videoTextureMutex.unlock();
}

sf::Vector2u VideoStream::getImageSize()
{
	return videoImage.getSize();
}

void VideoStream::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	videoTextureMutex.lock();
	target.draw(videoSprite, states);
	videoTextureMutex.unlock();
}
