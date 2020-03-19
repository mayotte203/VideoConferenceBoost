#pragma once
#include <SFML/Graphics.hpp>
#include <mutex>
#include "IPacketEndpoint.h"

class VideoStream : public sf::Drawable, public sf::Transformable, public IPacketEndpoint
{
public:
	VideoStream();
	virtual void handlePacket(const std::vector<uchar> packet, uchar packetType);
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	sf::Image videoImage;
	sf::Texture videoTexture;
	sf::Sprite videoSprite;
	mutable std::mutex videoTextureMutex;
};

