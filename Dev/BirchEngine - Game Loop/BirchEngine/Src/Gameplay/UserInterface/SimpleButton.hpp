#pragma once
#include "Src/ECS/components.hpp"

class SimpleButtonEntity : public Entity {
public:
	SimpleButtonEntity(Manager& m, int x, int y, int w, int h, ButtonMethod buttonMethod) : Entity(m) {
		SpatialComponent& sc = this->addComponent<SpatialComponent>(static_cast<float>(x), static_cast<float>(y), w, h, 1.0f);
		this->addComponent<ButtonComponent>(buttonMethod);
		this->addComponent<DrawComponent>(sc, SDL_Color{ 0,0,0,255 });
	}
};