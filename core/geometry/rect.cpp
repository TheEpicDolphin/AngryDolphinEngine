
#include "rect.h"

Rect::Rect(float x, float y, float width, float height) {
	this->origin = glm::vec2(x, y);
	this->size = glm::vec2(width, height);
}

Rect::Rect(glm::vec2 origin, glm::vec2 size) {
	this->origin = origin;
	this->size = size;
}