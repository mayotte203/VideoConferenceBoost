#include "MicrophoneStream.h"
#include "types.h"

MicrophoneStream::MicrophoneStream()
{
	initialize(1, 44100);
}

void MicrophoneStream::setSampleRate(unsigned int sampleRate)
{
	stop();
	initialize(1, sampleRate);
	play();
}

void MicrophoneStream::addSamples(std::vector<uint8_t> samples)
{
	std::scoped_lock<std::mutex> lock(samplesQueueMutex);
	samplesQueue.push(samples);
	if (samplesQueue.size() > 5)
	{
		samplesQueue.pop();
	}
}

void MicrophoneStream::handlePacket(const std::vector<uint8_t> packet, uint8_t packetType)
{
	addSamples(packet);
	if (getStatus() != sf::Sound::Playing)
	{
		play();
	}
}

bool MicrophoneStream::onGetData(Chunk& data)
{
	samplesQueueMutex.lock();
	if (samplesQueue.size() > 0)
	{
		currentBuffer.loadFromSamples(reinterpret_cast<sf::Int16*>(samplesQueue.front().data()), samplesQueue.front().size() / 2, 1, 44100);		
		samplesQueue.pop();
		samplesQueueMutex.unlock();
		data.samples = currentBuffer.getSamples();
		data.sampleCount = currentBuffer.getSampleCount();
		return true;
	}
	samplesQueueMutex.unlock();
	return false;
}

void MicrophoneStream::onSeek(sf::Time timeOffset)
{
	//ignore seek
}
