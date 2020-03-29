#pragma once
#include "GUIElement.h"
class GUITextField : public GUIElement
{
public:
	GUITextField();
	virtual void updateEvent(sf::Event& event);
	void setSize(const sf::Vector2f& size);
	void setPosition(const sf::Vector2f& position);
	void setGhostString(const sf::String& string);
	std::string getInputString();
private:
	void (*callbackFunction)(void) = nullptr;
	sf::Vector2f size;
	sf::Vector2f position;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	enum class State { Focused, NotFocused, Inactive };
	State state = State::NotFocused;
	void changeState(State state);
	sf::Text ghostText;
	sf::Text userInputText;
	sf::RectangleShape textFieldRectangle;
	sf::RectangleShape textEnterBar;
	sf::Font font;
	sf::String userInputString;
};

