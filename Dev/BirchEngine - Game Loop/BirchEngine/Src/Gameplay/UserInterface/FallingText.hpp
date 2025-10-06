#pragma once
#include "FloatingTextEntity.hpp"

class FallingTextEntity : public FloatingTextEntity {
public:
	FallingTextEntity(Manager& man, int x, int y, std::string text, std::string font, SDL_Color colour, Uint32 durationInMs) : FloatingTextEntity(man, x, y, text, font, colour) {
		this->addComponent<FadeAwayComponent>(durationInMs);
		this->addComponent<ProjectileComponent>(static_cast<float>(durationInMs), GameTime::fixedDeltaTime, Vector2D{ 0.0f,0.5f }, "text", nullptr);
	}
};