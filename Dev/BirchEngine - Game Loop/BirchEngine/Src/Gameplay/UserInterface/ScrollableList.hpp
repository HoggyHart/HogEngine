#pragma once
#include "Src/ECS/components.hpp"

class ScrollableList : public Entity {
	//all l
	ScrollableList(Manager& m) : Entity(m) {
		//add spatial component
		this->addComponent<SpatialComponent>();
	}
};