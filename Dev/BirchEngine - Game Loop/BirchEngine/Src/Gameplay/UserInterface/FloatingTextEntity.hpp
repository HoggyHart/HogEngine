#pragma once
#include "Src/ECS/components.hpp"

enum floatingTextType { textStatic, textStationary, textAnimated };
class FloatingTextEntity : public Entity {
protected:
public:
	FloatingTextEntity(Manager& man, int x, int y, std::string text, std::string font, SDL_Color& colour) : Entity(man) {
		this->addComponent<SpatialComponent>(static_cast<float>(x), static_cast<float>(y), 0, 0, 1.0f);
		this->addComponent<TextComponent>(CENTER, text, font, colour);
		this->addGroup(EntityGroup::groupGUIMenu); //change to layers::floatingText.
	}
	~FloatingTextEntity() {
		this->delGroup(EntityGroup::groupGUIMenu);
	}
};