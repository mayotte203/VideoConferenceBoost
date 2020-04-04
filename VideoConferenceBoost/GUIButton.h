#pragma once 
#include "GUIElement.h"
#include <functional>

class GUIButton : public GUIElement
{
public:
	GUIButton();
	void setCallbackFunction(std::function<void()> callbackFunction);
	virtual void updateEvent(sf::Event& event);
	void setSize(const sf::Vector2f& size);
	void setPosition(const sf::Vector2f& position);
	void setString(const sf::String& string);
private:
	std::function<void()> callbackFunction;
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

