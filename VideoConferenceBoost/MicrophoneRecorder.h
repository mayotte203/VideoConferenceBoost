#pragma once
#include <sfml/Audio.hpp>
#include "PacketTransceiver.h"

class MicrophoneRecorder : public sf::SoundRecorder
{
public:
	MicrophoneRecorder() = delete;
	MicrophoneRecorder(PacketTransceiver* packetTr);
	~MicrophoneRecorder();
private:
	PacketTransceiver* packetTr;
	virtual bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount);
};

