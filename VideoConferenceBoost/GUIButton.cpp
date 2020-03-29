#include "GUIButton.h"

GUIButton::GUIButton()
{
	font.loadFromFile("3966.ttf");
	buttonText.setFillColor(sf::Color::Black);
	buttonText.setFont(font);
	buttonRectangle.setOutlineThickness(3);
	buttonRectangle.setOutlineColor(sf::Color::Black);
	buttonRectangle.setFillColor(sf::Color::White);
}

void GUIButton::setCallbackFunction(void(*callbackFunction)(void))
{
	this->callbackFunction = callbackFunction;
}

void GUIButton::updateEvent(sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::MouseButtonPressed:
	{
		if (state == State::NotPressed
			&& event.mouseButton.button == sf::Mouse::Left
			&& sf::FloatRect(position, size).contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
		{
			changeState(State::Pressed);
			callbackFunction();
		}
		break;
	}
	case sf::Event::MouseButtonReleased:
	{
		if (state == State::Pressed
			&& event.mouseButton.button == sf::Mouse::Left)
		{
			changeState(State::NotPressed);
		}
		break;
	}
	}
}

void GUIButton::setSize(const sf::Vector2f& size)
{
	this->size = size;
	buttonRectangle.setSize(size);
	buttonText.setCharacterSize(size.y * 0.7f);
	buttonText.setPosition(position + sf::Vector2f((buttonRectangle.getLocalBounds().width - buttonText.getLocalBounds().width) / 2.0f,
		(buttonRectangle.getLocalBounds().height - buttonText.getLocalBounds().height) / 2.0f));
}

void GUIButton::setPosition(const sf::Vector2f& position)
{
	this->position = position;
	buttonRectangle.setPosition(position);
	buttonText.setPosition(position + sf::Vector2f((buttonRectangle.getLocalBounds().width-buttonText.getLocalBounds().width) / 2.0f, 0));
}

void GUIButton::setString(const sf::String& string)
{
	buttonText.setString(string);
	buttonText.setPosition(position + sf::Vector2f((buttonRectangle.getLocalBounds().width - buttonText.getLocalBounds().width) / 2.0f, 0));
}

void GUIButton::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(buttonRectangle, states);
	target.draw(buttonText, states);
}

void GUIButton::changeState(State state)
{
	switch (state)
	{
	case State::NotPressed:
	{
		buttonText.setFillColor(sf::Color::Black);
		buttonRectangle.setFillColor(sf::Color::White);
		break;
	}
	case State::Pressed:
	{
		buttonText.setFillColor(sf::Color::White);
		buttonRectangle.setFillColor(sf::Color::Black);
		break;
	}
	case State::Inactive:
	{
		break;
	}
	}
	this->state = state;
}
