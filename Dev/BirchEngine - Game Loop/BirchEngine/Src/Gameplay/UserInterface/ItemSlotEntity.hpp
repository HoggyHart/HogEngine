#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Utils/NodePubSub.hpp"
#include "Src/Gameplay/ItemStuff/Inventory.hpp"

class ItemSlotEntity : public Entity {
public:

	ItemSlotEntity(Manager& m, ItemSlot& mirror) : Entity(m) {
		this->addComponent<SpatialComponent>(0.0f, 0.0f, 32, 32, 2.0f);
		this->addComponent<SpriteComponent>("itemSlot");
		DataComponent<ItemSlot*>& cds = this->addComponent<DataComponent<ItemSlot*>>(&mirror);
		this->addComponent<CustomFunc>([this]() {this->updateContainer(); });
		UIComponent& cc = this->addComponent<UIComponent>(new GridLayout{1,1 });
		/*this->addComponent<DataComponent<Publisher<ItemSlotEntity*>>>("entity" + getId(), 0u, [this](Publisher<ItemSlotEntity*>& p) {return new ItemSlotEntity* { this }; });
		this->addComponent<DataComponent<Subscriber<ItemSlotEntity*>>>("entity" + getId(), [](ItemSlotEntity*& msg) {msg->updateContainer(); });*/

		//container component stores ItemStackEntity that is cds's itemStack;
	}

	void updateContainer() {
		DataComponent<ItemSlot*>& cds = this->getComponent<DataComponent<ItemSlot*>>();
		UIComponent& cc = this->getComponent<UIComponent>();

		//get entity in the slot (will be an ItemEntity)
		Entity* currentInSlot = cc.getChildren()[0];

		//if item slot is clear, make sure container is clear too, then return.
		if (cds.data->isEmpty()) {
			cc.clear(false); return;
		}
		//if item in slot is already the one in the container, return.
		if (cds.data->stack->item.abilities.abilitySetOwner == currentInSlot && cds.data->stack->item.abilities.abilitySetOwner->hasComponent<SpatialComponent>()) return;

		//else slot is not clear AND it isnt the same as last check
		cc.clear(false); //false is passed to not destroy the ItemStackEntity since it should be handled elsewhere by whatever caused this item slot to have a different item
		static_cast<ItemStackEntity*>(cds.data->stack->item.abilities.abilitySetOwner)->addToDisplay(0, 0, 0, 0, 0); //add item to display
		cc.addToContainer(*cds.data->stack->item.abilities.abilitySetOwner); //add itementity to container
	}
};