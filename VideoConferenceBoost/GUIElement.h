#pragma once
#include <sfml/Window/Event.hpp>
#include <sfml/Graphics.hpp>
class GUIElement : public sf::Drawable
{
public:
	void setFocus(bool focus);
	bool isFocused();
	void setActive(bool active);
	bool isActive();
	virtual void updateEvent(sf::Event& event) = 0;
private:
	bool focus = false;
	bool active = false;
};

