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
	void addSamples(std::vector<uchar> samples);
	virtual void handlePacket(const std::vector<uchar> packet, uchar packetType);
private:
	std::mutex samplesQueueMutex;
	std::queue<std::vector<uchar>> samplesQueue;
	//std::vector<sf::Int16> currentBuffer;
	sf::SoundBuffer currentBuffer;
	virtual bool onGetData(Chunk& data);
	virtual void onSeek(sf::Time timeOffset);
};

