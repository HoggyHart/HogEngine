#include "Src/Gameplay/ItemStuff/ItemEntity.hpp"

#include "Src/Gameplay/ItemStuff/Inventory.hpp"

extern BaseItem baseballBat{ "Baseball Bat","Swing, batter batter!", "baseballBat" };
extern BaseItem gun{ "GUN","Pow!","bulletPNG" };

std::function<void(Entity*, Entity*, Uint8&)> ItemStackEntity::pickUpItem = [](Entity* itemStack, Entity* interacter, Uint8& unused) {
	ItemStack& s = itemStack->getComponent<DataComponent<ItemStack>>().data;

	//if interacter has a hotbar, attempt to fill it
	if (interacter->hasComponent<DataComponent<Hotbar>>() //if has hotbar
		&& interacter->getComponent<DataComponent<Hotbar>>().data.fillNextEmptySlot(s) //and succesfully added to hotbar
		//FIX: should be more Inventory::addItemStack() (add to existing stack if possible)
		) {
		static_cast<ItemStackEntity*>(itemStack)->removeFromWorld(); //remove from world
	}
	//else if adding to hotbar failed, try adding to inventory
	else if (interacter->hasComponent<DataComponent<Inventory>>()  //if has inventory
		&& interacter->getComponent<DataComponent<Inventory>>().data.addItemStack(s)) { //and succesfully added to inventory
		static_cast<ItemStackEntity*>(itemStack)->removeFromWorld();
	}
	//else failed to add to hotbar AND inventory
	//do nothing
	};