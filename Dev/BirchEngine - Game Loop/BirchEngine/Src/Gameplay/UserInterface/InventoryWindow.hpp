#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/ItemStuff/Inventory.hpp"
#include "ItemSlotEntity.hpp"
class InventoryEntity : public Entity {
public:
	InventoryEntity(Manager& m, Inventory*& inv) : Entity(m) {
		this->makeDisplayEntity();

		std::size_t slots = inv->getItems().size();
		const static int slotHeight = 64;
		const static int slotWidth = 64;
		this->addComponent<SpatialComponent>(Game::camera.displaySize.w * 0.2, Game::camera.displaySize.h * 0.2, Game::camera.displaySize.w * 0.6, Game::camera.displaySize.h * 0.6, 1.0f);

		UIComponent& cc = this->addComponent<UIComponent>(new GridLayout{ 10,4 });

		for (int i = 0; i < 40; ++i) {
			Entity& e{ entityManager.queueAddEntity<ItemSlotEntity>(std::reference_wrapper(inv->getItems()[i])) };
			cc.addToContainer(e);
		}

		InputResponseMethod t = [](Entity* ent, bool& a, bool& b) {
			if (ent->hasGroup(EntityGroup::groupGameHUD)) static_cast<InventoryEntity*>(ent)->hide();
			else  static_cast<InventoryEntity*>(ent)->show();
			b = false;
			};

		this->addComponent<InputController>().addControl(InputResponse{ t,false }, KeyComboInput{ {SDLK_i},true });
	}
	void show() { this->addGroup(EntityGroup::groupGameHUD); }
	void hide() { this->delGroup(EntityGroup::groupGameHUD); }
};