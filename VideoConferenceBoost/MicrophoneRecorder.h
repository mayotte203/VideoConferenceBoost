#pragma once
#include <sfml/Audio.hpp>
#include "PacketRouter.h"

class MicrophoneRecorder : public sf::SoundRecorder
{
public:
	MicrophoneRecorder() = delete;
	MicrophoneRecorder(PacketRouter& packetRouter);
	~MicrophoneRecorder();
private:
	PacketRouter* packetRouter;
	virtual bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount);
};

