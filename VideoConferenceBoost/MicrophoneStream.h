#pragma once
#include <sfml/Audio.hpp>
#include "types.h"
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>
#include "IPacketEndpoint.h"
#include "PacketRouter.h"

class MicrophoneStream : public sf::SoundStream, public IPacketEndpoint
{
public:
	MicrophoneStream();
	void addSamples(std::vector<uint8_t> samples);
	virtual void handlePacket(const std::vector<uint8_t> packet, uint8_t packetType);
private:
	std::mutex samplesQueueMutex;
	std::queue<std::vector<uint8_t>> samplesQueue;
	//std::vector<sf::Int16> currentBuffer;
	sf::SoundBuffer currentBuffer;
	virtual bool onGetData(Chunk& data);
	virtual void onSeek(sf::Time timeOffset);
};

