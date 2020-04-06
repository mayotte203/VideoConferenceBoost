#include "MicrophoneRecorder.h"
#include <types.h>

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
	std::vector<uint8_t> packet = std::vector<uint8_t>(reinterpret_cast<uint8_t*>(const_cast<sf::Int16*>(samples)), reinterpret_cast<uint8_t*>(const_cast<sf::Int16*>(samples + sampleCount)));
	packetRouter->send(std::move(packet), PacketType::Sound);
	return true;
}
