#pragma once
#include "Src/ECS/components.hpp"
class HighlightFrameEntity : public Entity {
public:
	HighlightFrameEntity(Manager& m, int x, int y, int w, int h, bool ingame) : Entity(m) {
		if (!ingame) { this->makeDisplayEntity(); }
		SpatialComponent& sc = this->addComponent<SpatialComponent>(static_cast<float>(x), static_cast<float>(y), w, h, 1.0f);
		this->addComponent<DrawComponent>(sc, SDL_Color{ 255,255,255,255 });
		this->addGroup(EntityGroup::groupGameHUD);
	}
	~HighlightFrameEntity() {
		this->delGroup(EntityGroup::groupGameHUD);
	}
};