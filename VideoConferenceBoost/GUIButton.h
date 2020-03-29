#pragma once
#include <SFML/Graphics.hpp>
#include "GUIElement.h"

class GUIButton : public GUIElement
{
public:
	GUIButton();
	void setCallbackFunction(void (*callbackFunction)(void));
	virtual void updateEvent(sf::Event& event);
	void setSize(const sf::Vector2f& size);
	void setPosition(const sf::Vector2f& position);
	void setString(const sf::String& string);
private:
	void (*callbackFunction)(void) = nullptr;
	sf::Vector2f size;
	sf::Vector2f position;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	enum class State{Pressed, NotPressed, Inactive};
	State state = State::NotPressed;
	void changeState(State state);
	sf::Text buttonText;
	sf::RectangleShape buttonRectangle;
	sf::Font font;
};

