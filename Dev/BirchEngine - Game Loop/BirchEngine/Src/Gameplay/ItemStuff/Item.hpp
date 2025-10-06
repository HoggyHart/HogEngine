#pragma once
#include "Src/Utils/utils.hpp"
#include "Src/Gameplay/AbilityStuff/Abilities.hpp"

//determines when the item effect is applied

class Entity;

//FIX: not sure base item is needed. could probably just go straight to Item and have this stuff in there
class BaseItem {
	//Item stuff
public:
	std::string itemName{ "Unknown Item" };
	std::string itemDescription{ "Missing description." };

	BaseItem(std::string name, std::string description, std::string spriteID) : itemName(name), itemDescription(description), spriteID(spriteID) {
	}

	//Entity stuff
private:
	std::string spriteID;
public:
	std::string getSpriteID() {
		return spriteID;
	}
};

//UNFINISHED; I think this could function similar to entities, with a component system for durability component, enchantment component, etc.

class Item {
public:
	//entity that holds this item
	Entity* itemEntity{ nullptr };

	Entity* holder{ nullptr }; //indicates entity that currently has this item EQUIPPED AND IN USE
	AbilitySet abilities{};
	BaseItem& base;

public:
	Item(BaseItem& base) : base(base) {
	}
	Item(Item& item)
		: base(item.base),
		abilities(item.abilities)
	{
	}

	void bindToEntity(Entity& e) { itemEntity = &e; }
	void equipToEntity(Entity* e) {
		//FIX: entity HOLDING item does not necessarily mean one using abilities,
		// i.e.:
		//	EntityA holds item, and grants its abilities to EntityB, then EntityB can use and pass the abilities to EntityC, etc. etc.
		// this means decoupling Item::holder and AbilitySet::user here
		// not really sure exactly how i would do that without breaking everything
		holder = e;
		abilities.equipToEntity(holder);
	}

	void addUse(Ability& a) {
		abilities.addAbility(a);
	}
	void addUse(const Ability& a) {
		abilities.addAbility(a);
	}
	void addUses(AbilitySet abilities) {
		for (std::pair<const std::string, Ability>& a : abilities.abilities) abilities.addAbility(a.second);
	}
};

struct ItemStack {
	Item item;
	unsigned int stackSize{ 1 };

	ItemStack(Item& i, unsigned int n) : item(i), stackSize(n) {}
};