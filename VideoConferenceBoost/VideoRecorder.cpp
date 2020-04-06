#include "VideoRecorder.h"
#include <vector>
#include <types.h>

VideoRecorder::VideoRecorder(PacketRouter& packetRouter)
{
	this->packetRouter = &packetRouter;

#ifdef SINGLE_PC
	videoImage.create(640, 480);
#else
	webcamVC = cv::VideoCapture(0);
	if (!webcamVC.isOpened())
	{
		throw(std::exception("Webcam Error"));
	}
	cv::Mat webcamMat, RGBAMat;
	webcamVC.read(webcamMat);
	if (webcamMat.empty())
	{
		throw(std::exception("Webcam Error"));
	}
	cv::cvtColor(webcamMat, RGBAMat, cv::COLOR_BGR2RGBA);
	videoImage.create(RGBAMat.cols, RGBAMat.rows, RGBAMat.ptr());
#endif // SINGLE_PC

	videoTexture.loadFromImage(videoImage);
	videoTexture.setSmooth(true);
	videoSprite.setTexture(videoTexture);
	
	videoRecorderThread = std::thread(&VideoRecorder::videoRecorderThreadFunction, this);
}

VideoRecorder::~VideoRecorder()
{
	recording = false;
	if (videoRecorderThread.joinable())
	{
		videoRecorderThread.join();
	}
}

sf::Vector2u VideoRecorder::getImageSize()
{
	return videoImage.getSize();
}

void VideoRecorder::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	videoTextureMutex.lock();
	target.draw(videoSprite, states);
	videoTextureMutex.unlock();
}

void VideoRecorder::videoRecorderThreadFunction()
{
	cv::Mat webcamMat, RGBAMat;
	std::vector<uint8_t> jpegBuffer;

#ifdef SINGLE_PC
	sf::Font font;
	sf::Text text;
	sf::RenderTexture renderTexture;
	renderTexture.create(640, 480);
	font.loadFromFile("3966.ttf");
	text.setFillColor(sf::Color::Black);
	text.setFont(font);
	text.setCharacterSize(30);
	text.setPosition(50, 200);
#endif // SINGLE_PC	

	while (recording)
	{
#ifdef SINGLE_PC
		std::stringstream ss;
		ss << time(0) << " " << rand() << std::endl;
		text.setString(ss.str());
		renderTexture.clear(sf::Color::White);
		renderTexture.draw(text);
		renderTexture.display();
		videoImage = renderTexture.getTexture().copyToImage();
		uint8_t* pixels = reinterpret_cast<uint8_t*>(const_cast<sf::Uint8*>(videoImage.getPixelsPtr()));
		cv::Mat imageMat(cv::Size(videoImage.getSize().x, videoImage.getSize().y), CV_8UC4, pixels);
		cv::cvtColor(imageMat, RGBAMat, cv::COLOR_RGBA2BGR);
		cv::imencode(".jpg", RGBAMat, jpegBuffer);
#else
		webcamVC.read(webcamMat);
		cv::cvtColor(webcamMat, RGBAMat, cv::COLOR_BGR2RGBA);
		videoImage.create(RGBAMat.cols, RGBAMat.rows, RGBAMat.ptr());
		cv::imencode(".jpg", std::move(webcamMat), jpegBuffer, std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 70});
#endif // SINGLE_PC	
		packetRouter->send(std::move(jpegBuffer), PacketType::Image);
		videoTextureMutex.lock();
		videoTexture.loadFromImage(videoImage);
		videoTextureMutex.unlock();
	}
#ifdef SINGLE_PC
#else
	webcamVC.release();
#endif // SINGLE_PC
}
