#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/UserInterface/WindowEntity.hpp"
class EntitySpawnMenu : public WindowEntity {
	EntitySpawnMenu(Manager& m, std::vector<Entity*>& entities) : WindowEntity(m, Game::camera.displaySize.w / 2, Game::camera.displaySize.h / 2, 200, 200, "Entity Spawner Menu") {
		for (Entity*& e : entities) {
			this->windowParts["wa"]->getComponent<UIComponent>().addToContainer(e);
		}
	}
};