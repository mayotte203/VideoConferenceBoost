#pragma once
#include <sfml/Audio.hpp>
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include "IPacketEndpoint.h"
#include "PacketRouter.h"
#include "types.h"

class MicrophoneStream : public sf::SoundStream, public IPacketEndpoint
{
public:
	MicrophoneStream();
	void setSampleRate(unsigned int sampleRate);
	void addSamples(std::vector<uint8_t> samples);
	virtual void handlePacket(const std::vector<uint8_t> packet, uint8_t packetType);
private:
	std::mutex samplesQueueMutex;
	std::queue<std::vector<uint8_t>> samplesQueue;
	sf::SoundBuffer currentBuffer;
	virtual bool onGetData(Chunk& data);
	virtual void onSeek(sf::Time timeOffset);
};

