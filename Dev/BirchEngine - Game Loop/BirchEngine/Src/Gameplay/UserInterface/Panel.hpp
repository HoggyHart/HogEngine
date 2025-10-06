#pragma once
#include "Src/ECS/components.hpp"

//All other UI Entities must be added within a panel

//FIX: sort out creation, free layout, grid layout, columns, rows, etc.
		//should it default to free and then have a setLayout call be needed?
class Panel : public Entity {
	Panel(Manager& m, int x, int y, int w, int h) : Entity(m) {
		this->makeDisplayEntity();

		this->addComponent<SpatialComponent>(x, y, w, h, 1.0f);
	}

	template<typename LayoutType, typename std::enable_if_t<std::is_base_of<Layout, LayoutType>::value, int> = 0>
	void setLayout(LayoutType layout) {
		this->getComponent<UIComponent>().setLayout(layout);
	}
};