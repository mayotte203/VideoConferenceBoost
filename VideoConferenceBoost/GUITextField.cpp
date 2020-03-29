#include "GUITextField.h"

GUITextField::GUITextField()
{
	font.loadFromFile("3966.ttf");
	ghostText.setFillColor(sf::Color(128, 128, 128));
	ghostText.setFont(font);
	userInputText.setFillColor(sf::Color::Black);
	userInputText.setFont(font);
	textFieldRectangle.setOutlineThickness(3);
	textFieldRectangle.setOutlineColor(sf::Color::Black);
	textFieldRectangle.setFillColor(sf::Color::White);
	textEnterBar.setSize(sf::Vector2f(2.0f, size.y * 0.7f));
	textEnterBar.setFillColor(sf::Color::Black);
	textFieldRectangle.setPosition(position);
	userInputText.setPosition(position + sf::Vector2f(5.0f, 0));
	ghostText.setPosition(position + sf::Vector2f(5.0f, 0));
	textEnterBar.setPosition(sf::Vector2f(userInputText.getGlobalBounds().left + userInputText.getGlobalBounds().width + 1.0f, position.y + size.y * 0.15f));
}

void GUITextField::updateEvent(sf::Event& event)
{
	switch (event.type)
	{
	case sf::Event::TextEntered:
	{
		if (state == State::Focused)
		{
			if (event.text.unicode == '\b')
			{
				if (userInputString.getSize() > 0)
				{
					userInputString.erase(userInputString.getSize() - 1, 1);
				}
			}
			else
			{
				userInputString += event.text.unicode;
			}
			userInputText.setString(userInputString);
			if (userInputText.getLocalBounds().width + 5.0f > size.x)
			{
				if (userInputString.getSize() > 0)
				{
					userInputString.erase(userInputString.getSize() - 1, 1);
				}
				userInputText.setString(userInputString);
			}
			textEnterBar.setPosition(sf::Vector2f(userInputText.getGlobalBounds().left + userInputText.getGlobalBounds().width + 1.0f, position.y + size.y * 0.15f));
		}
		break;
	}
	case sf::Event::MouseButtonPressed:
	{
		if (sf::FloatRect(position, size).contains(sf::Vector2f(event.mouseButton.x, event.mouseButton.y)))
		{
			if (state == State::NotFocused && event.mouseButton.button == sf::Mouse::Left)
			{
				changeState(State::Focused);
			}
		}
		else
		{
			changeState(State::NotFocused);
		}
		break;
	}
	}
}

void GUITextField::setSize(const sf::Vector2f& size)
{
	textFieldRectangle.setSize(size);
	userInputText.setCharacterSize(size.y * 0.7f);
	ghostText.setCharacterSize(size.y * 0.7f);
	userInputText.setPosition(position + sf::Vector2f(5.0f, 0));
	ghostText.setPosition(position + sf::Vector2f(5.0f, 0));
	textEnterBar.setSize(sf::Vector2f(2.0f, size.y * 0.7f));
	textEnterBar.setPosition(sf::Vector2f(userInputText.getGlobalBounds().left + userInputText.getGlobalBounds().width + 1.0f, position.y + size.y * 0.15f));
	this->size = size;
}

void GUITextField::setPosition(const sf::Vector2f& position)
{
	textFieldRectangle.setPosition(position);
	userInputText.setPosition(position + sf::Vector2f(5.0f, 0));
	ghostText.setPosition(position + sf::Vector2f(5.0f, 0));
	textEnterBar.setPosition(sf::Vector2f(userInputText.getGlobalBounds().left + userInputText.getGlobalBounds().width + 1.0f, position.y + size.y * 0.15f));
	this->position = position;
}

void GUITextField::setGhostString(const sf::String& string)
{
	ghostText.setString(string);
}

std::string GUITextField::getInputString()
{
	return userInputString.toAnsiString();
}

void GUITextField::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(textFieldRectangle);
	if (state == State::NotFocused && userInputString.isEmpty())
	{
		target.draw(ghostText);
	}
	else
	{
		target.draw(userInputText);
	}
	if (state == State::Focused)
	{
		target.draw(textEnterBar);
	}
}

void GUITextField::changeState(State state)
{
	switch (state)
	{
	case State::Focused:
	{
		break;
	}
	case State::NotFocused:
	{
		break;
	}
	}
	this->state = state;
}
