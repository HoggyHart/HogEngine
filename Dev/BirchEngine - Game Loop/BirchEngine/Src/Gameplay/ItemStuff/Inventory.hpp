#pragma once
#include "Src/Gameplay/ItemStuff/Item.hpp"
class Inventory;

class ItemSlot {
public:
	Inventory* restOfInventory{ nullptr };
	ItemStack* stack{ nullptr };
public:
	ItemSlot() {};

	void setInventory(Inventory& i) { restOfInventory = &i; }

	ItemStack* replaceItem(ItemStack* newStack) {
		//store old stack
		ItemStack* temp = stack;

		//unequip old item from entity (if necessary)
		if (stack) stack->item.equipToEntity(nullptr);
		//then take in new stack
		stack = newStack;

		//return old stack
		return temp;
		//POTFIX: MUST TAKE CARE OF RETURN VALUE. OTHERWISE IT MAY CAUSE A MEMORY LEAK
	}

	bool isEmpty() const { return stack == nullptr; }
};

typedef std::array<ItemSlot, 40> ItemStackList;
class Inventory {
private:
	ItemStackList collectedItems{};
public:
	Inventory() {
		for (ItemSlot& s : collectedItems) s.setInventory(*this);
	};
	~Inventory() {
		//delete items maybe? depends how items are handled on inventory destruction (dropped, deleted, transferred? this should be handled before destruction tho)
	}
	//returns whether item was able to added or not
	bool addItemStack(ItemStack& stack) {
		std::size_t i{ 0 };
		ItemSlot* firstEmptySlot{ nullptr };
		for (ItemSlot& p : collectedItems) {
			//if this slot is the first clear slot found, mark this slot as the one to fill if no stack is found.
			if (p.isEmpty() && firstEmptySlot == nullptr) firstEmptySlot = &p;
			//else if this item matches the one being added //FIX: replace with non-pointer comparison i.e. actual stats and abilities comparison
			else if (!p.isEmpty() && (&p.stack->item == &stack.item)) { //FIX: i dont think this ever works. create Item == Item method
				p.stack->stackSize += stack.stackSize;
				return true;
			}
		}
		//if no stack was found, add into clear slot
		if (firstEmptySlot) {
			firstEmptySlot->replaceItem(&stack);
			return true;
		}
		//if unable to be added to existing or new stack, return fail to pick up
		return false;
	};
	bool swapSlots(ItemSlot& otherSlot) {
	}
	ItemStackList& getItems() {
		return collectedItems;
	}
};

//CONTINUE: FIND PROPER WAY TO HANDLE CURRENTLY EQUIPPED STUFF VS ENTITY ASSIGNED STUFF I.E. ABILITYSETS.
//2 types of hotbar: only B needs special stuff i think. option A is just a visual aid really
//	A) select hotbar (a la risk of rain 2, display options to choose from, can use multiple objects at once)
//	B) equip hotbar (a la minecraft, select an object from the hotbar to hold and make use of)
//B is option A but with the added function of calling obj.unequip() and obj.equip() when using ability
//FIX: to implement option A properly (i.e. allow abilities to be in the slot) Hotbar must be detached from ItemEntities.
// ItemEntities and Abilities must also be derived from a class like Equippable so that equip/unequip can be called on them both should the need arise
//This is a selection-type hotbar, items are equipped when they are selected by pressing numbers 1-5
const std::size_t hotbarSize = 5;
typedef std::array<ItemSlot, hotbarSize> HotbarSlots;
class Hotbar {
	std::size_t currentSelectedSlot{ 0 };
public:

	Entity* hotbarOwner{ nullptr };
	HotbarSlots slots{};

	Hotbar() {}
	ItemSlot& getCurrentSlot() { return slots[currentSelectedSlot]; }
	ItemStack* swapSlotContents(std::size_t slotNum, ItemStack& s) {
		//if not a slot, return the item trying to get in
		if (slotNum >= hotbarSize) return &s;
		//if it is a slot, attempt to replace item, return the result.
		return slots[slotNum].replaceItem(&s);
	}
	bool fillNextEmptySlot(ItemStack& s) {
		for (std::size_t i = 0; i < slots.size(); ++i) {
			ItemSlot& slot = slots[i];
			//if this slot is filled, skip to next slot
			if (!slot.isEmpty()) continue;
			//if it is clear, replace the clear contents with the item and return true (result of slot.replaceItem() on clear slot is nullptr, !nullptr == true)
			else {
				//if filling currently selected slot, equip item.
				if (i == currentSelectedSlot) s.item.equipToEntity(hotbarOwner);
				return !slot.replaceItem(&s);
			}
		}
		//return false, i.e. no clear slot found
		return false;
	}
	void switchToSlot(std::size_t slotNum) {
		//if hotbar isnt big enouhg for this slot, or if it is swapping tot he slot its already on
		if (slotNum >= hotbarSize || slotNum == currentSelectedSlot) return;

		//if something equipped, unequip it
		if (!slots[currentSelectedSlot].isEmpty()) slots[currentSelectedSlot].stack->item.equipToEntity(nullptr);
		//swap to slot number
		currentSelectedSlot = slotNum;
		//equip item in that slot
		//FIX/CONTINUE: at this point, item holder is NULL, ability entityManager and user are NULL
		if (!slots[currentSelectedSlot].isEmpty()) slots[currentSelectedSlot].stack->item.equipToEntity(hotbarOwner);
	}

	//these may have to be expanded upon to deactivate their effects? not sure
	void unequipFromEntity() {
		if (!slots[currentSelectedSlot].isEmpty()) slots[currentSelectedSlot].stack->item.equipToEntity(nullptr);
		hotbarOwner = nullptr;
	}
	//POTENTIAL LATER NEEDED FIX: MAKE SURE ABILITYSETS ARENT EQUIPPED TO ENTITIES UNLESS THEY ARE ACTIVELY IN USE. I.E. IF -1 ACTIVE ITEMS ARE PICKED UP BY THE USER, MAKE SURE NOT EVERY ITEM IS EQUIPPED
	void equipToEntity(Entity& e) {
		hotbarOwner = &e;
	}
};