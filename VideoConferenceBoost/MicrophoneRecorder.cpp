#include "MicrophoneRecorder.h"

MicrophoneRecorder::MicrophoneRecorder(PacketRouter& packetRouter)
{
	this->packetRouter = &packetRouter;
}

MicrophoneRecorder::~MicrophoneRecorder()
{
	stop();
}

bool MicrophoneRecorder::onProcessSamples(const sf::Int16* samples, std::size_t sampleCount)
{
	std::vector<uchar> packet = std::vector<uchar>(reinterpret_cast<uchar*>(const_cast<sf::Int16*>(samples)), reinterpret_cast<uchar*>(const_cast<sf::Int16*>(samples + sampleCount)));
	packetRouter->send(std::move(packet), PacketType::Sound);
	return true;
}
