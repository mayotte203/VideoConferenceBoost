#include "MicrophoneRecorder.h"

MicrophoneRecorder::MicrophoneRecorder(PacketTransceiver* packetTr)
{
	this->packetTr = packetTr;
}

MicrophoneRecorder::~MicrophoneRecorder()
{
	stop();
}

bool MicrophoneRecorder::onProcessSamples(const sf::Int16* samples, std::size_t sampleCount)
{
	std::vector<uchar> packet = std::vector<uchar>(reinterpret_cast<uchar*>(const_cast<sf::Int16*>(samples)), reinterpret_cast<uchar*>(const_cast<sf::Int16*>(samples + sampleCount)));
	packet.push_back(PacketType::Sound);
	packetTr->sendPacket(std::move(packet));
	return true;
}
