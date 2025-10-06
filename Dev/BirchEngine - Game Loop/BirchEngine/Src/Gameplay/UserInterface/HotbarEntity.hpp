#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/ItemStuff/Inventory.hpp"
#include "ItemSlotEntity.hpp"
//THIS Hotbar has CustomDataStructureComponent<Inventory>, UIComponent(5,1), InputControllerComponent
// Container contains 5 ItemSlotEntities
//	ItemSlotEntities have CustomData<ItemSlot>, UIComponent (ItemEntities, 1,1), DrawComponent(1 square)
//ALL have spatialcomponent (duh)
//
//AFTER implementing an item hotbar, consider ability hotbar/
class HotbarEntity : public Entity {
public:
	HotbarEntity(Manager& m, Hotbar*& h) : Entity(m) {
		this->makeDisplayEntity();
		std::size_t slots = h->slots.size();
		const static int slotHeight = 64;
		const static int slotWidth = 64;
		SpatialComponent& hsc = this->addComponent<SpatialComponent>(Game::camera.displaySize.w / 2.0f - slots * slotWidth / 2.0f, Game::camera.displaySize.h - slotHeight, slots * slotWidth, slotHeight, 1.0f);
		Hotbar* hb = this->addComponent<DataComponent<Hotbar*>>(h).data;
		InputController& ic = this->addComponent<InputController>();

		//add highlighting square to show current slot
		Entity* hle = &entityManager.queueAddEntity<Entity>();
		hle->addComponent<SpatialComponent>(hsc.position.x, hsc.position.y, 64, 64, 1.0f);
		hle->addComponent<DrawComponent>(hle->getComponent<SpatialComponent>(), SDL_Color{ 255,255,255,255 });
		hle->makeDisplayEntity();

		UIComponent& cc = this->addComponent<UIComponent>(new GridLayout{ static_cast<int>(slots),1});
		//add itemslots to container and add hotbar slot switching controls
		for (std::size_t i = 0; i < slots; ++i) {
			//create entity of item slot
			Entity* e{ &entityManager.queueAddEntity<ItemSlotEntity>(std::reference_wrapper(h->slots[i])) };
			cc.addToContainer(*e);
			//create method to switch to this slot, then add control with associated number key
			InputResponseMethod m = [hle, e, hb, i](Entity* ent, bool& a, bool& b) {
				hb->switchToSlot(i);
				hle->getComponent<SpatialComponent>().position = e->getComponent<SpatialComponent>().position;
				};
			ic.addControl(InputResponse{ m,false }, KeyComboInput{ {static_cast<SDL_Keycode>(49 + i)},true });
		}

		//add drop item control;
		InputResponseMethod dropMethod = [this](Entity* ent, bool& a, bool& b) {
			Hotbar*& hb = this->getComponent<DataComponent<Hotbar*>>().data;
			std::vector<Entity*>& itemSlots = this->getComponent<UIComponent>().getChildren();

			if (hb->getCurrentSlot().isEmpty()) return;
			ItemStack* droppedItem = hb->getCurrentSlot().replaceItem(nullptr);
			SpatialComponent& holderSC = hb->hotbarOwner->getComponent<SpatialComponent>();
			static_cast<ItemStackEntity*>(droppedItem->item.abilities.abilitySetOwner)->removeFromDisplay();
			static_cast<ItemStackEntity*>(droppedItem->item.abilities.abilitySetOwner)->addToWorld(holderSC.position.x, holderSC.position.y, 32, 32, 1.0f);
			//FIX/CONTINUE: potential problem with display entities and interaction with this
			//	when dropped item will get added to world BUT if the item slot hasnt updated yet it will still get caught by the
			//	slots UIComponent resizing and stuff
			b = false; //prevent repeat call
			};
		ic.addControl(InputResponse{ dropMethod,false }, KeyComboInput{ {SDLK_q},true });

		this->addGroup(EntityGroup::groupGameHUD);
		hle->addGroup(EntityGroup::groupGameHUD);
	}
};