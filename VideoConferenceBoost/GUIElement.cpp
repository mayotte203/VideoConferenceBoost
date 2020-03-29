#include "GUIElement.h"

void GUIElement::setFocus(bool focus)
{
	this->focus = focus;
}

bool GUIElement::isFocused()
{
	return focus;
}

void GUIElement::setActive(bool active)
{
	this->active;
}

bool GUIElement::isActive()
{
	return active;
}
