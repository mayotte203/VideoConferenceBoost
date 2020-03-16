#pragma once
#include <sfml/Audio.hpp>
#include "types.h"
#include <vector>
#include <queue>
#include <mutex>
#include <iostream>

class MicrophoneStream : public sf::SoundStream
{
public:
	MicrophoneStream();
	void addSamples(std::vector<uchar> samples);
private:
	std::mutex samplesQueueMutex;
	std::queue<std::vector<uchar>> samplesQueue;
	//std::vector<sf::Int16> currentBuffer;
	sf::SoundBuffer currentBuffer;
	virtual bool onGetData(Chunk& data);
	virtual void onSeek(sf::Time timeOffset);
};

