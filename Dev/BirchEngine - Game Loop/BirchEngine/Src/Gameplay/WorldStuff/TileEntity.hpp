#pragma once
#include "Src/ECS/components.hpp"

class TileEntity : public Entity {
public:
	TileEntity(Manager& man, int srcX, int srcY, int x, int y, int tsize, float tscale, std::string tileTexID) : Entity(man) {
		this->addComponent<SpatialComponent>(static_cast<float>(x), static_cast<float>(y), tsize, tsize, tscale);
		this->addComponent<TileComponent>(srcX, srcY, x, y, tsize, tscale, tileTexID);
		this->addGroup(EntityGroup::groupMap);
	}
};