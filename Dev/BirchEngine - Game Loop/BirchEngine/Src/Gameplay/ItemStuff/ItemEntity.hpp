#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/ItemStuff/Item.hpp"
#include <random>

//TMEPORARY. CREATE ACTUAL PLACE FOR ALL SIMEPL ITEMS TO BE HELD
extern BaseItem baseballBat;
extern BaseItem gun;

static int testVar = 0;

//FIX: creating this item will leave a small memory leak when deleted (i think). after adding 1000000 ItemStackEntities and then deleting them all AND resising manager vectors, it leaves about 30-56mb extra memory in use
//	this means each ItemEntity leaves behind ~ 30-56 bytes of leaked memory
// since its variable i wonder if it is just cached memory or maybe fragmentation memory
// on a similar note, adding 1000000 regular Entities and then deleting them all AND resising manager vectors leaves ~3mb of leftover memory waste.
class ItemStackEntity : public Entity {
private:
	static std::function<void(Entity* interactableEntity, Entity* interacter, Uint8& interactionStage)> pickUpItem;

	std::string spriteID{ "baseballBat" };
public:

	Entity* holder{ nullptr };

	ItemStackEntity(Manager& man, Item& i) : Entity(man)
	{
		//ALT/UNRELATEDFIX: items should use AbilityIDs so when written to file they can be read back and get their abilities back (need to store effectstats also maybe)
		//						 abilities should be rejigged a little bit, i.e. the lambda methods are stored as const and given abilityIDs, everything else can be file-read i think

		Item& item = this->addComponent<DataComponent<ItemStack>>(i, 1u).data.item;
		item.abilities.changeOwner(this);
		item.bindToEntity(*this);
		spriteID = this->getComponent<DataComponent<ItemStack>>().data.item.base.getSpriteID();
	};

	void addToWorld(
		float x, float y, float w, float h, float s //SpatialComponent args
	) {
		this->makeGameEntity();
		if (!this->hasComponent<SpatialComponent>()) {
			this->addComponent<SpatialComponent>();
			this->addComponent<SpriteComponent>(spriteID);

			this->addComponent<ColliderComponent>(HitboxShape::Circle,0u);
			this->addGroup(EntityGroup::groupGlobalColliders);
			this->addGroup(EntityGroup::groupItems);

			InteractableComponent& ic = this->addComponent<InteractableComponent>();
			ic.setInteractionResult(pickUpItem);
			testVar = 1;
		}

		SpatialComponent& sc = this->getComponent<SpatialComponent>();
		sc.position.x = x;
		sc.position.y = y;
		sc.width = w;
		sc.height = h;
		sc.scale = s;
	}
	void removeFromWorld() {
		if (this->hasComponent<SpatialComponent>()) {
			this->delComponent<SpatialComponent>();
			this->delComponent<SpriteComponent>();

			this->delComponent<ColliderComponent>();
			this->delGroup(EntityGroup::groupGlobalColliders);

			this->delComponent<InteractableComponent>();
			this->delGroup(EntityGroup::groupItems);
		}
	}

	void addToDisplay(
		float x, float y, float w, float h, float s //SpatialComponent args
	) {
		this->makeDisplayEntity();
		if (!this->hasComponent<SpatialComponent>()) {
			this->addComponent<SpatialComponent>();
			this->addComponent<SpriteComponent>(spriteID);
			this->addComponent<UIComponent>();
			this->addGroup(EntityGroup::groupGameHUD);
			//FIX: add buttoncomponent too
		}

		SpatialComponent& sc = this->getComponent<SpatialComponent>();
		sc.position.x = x;
		sc.position.y = y;
		sc.width = w;
		sc.height = h;
		sc.scale = s;
	}
	void removeFromDisplay() {
		if (this->hasComponent<SpatialComponent>()) {
			this->delComponent<SpatialComponent>();
			this->delComponent<SpriteComponent>();
			this->delComponent<UIComponent>();
			this->delGroup(EntityGroup::groupGameHUD);
			//remove buttoncomponent too when implemented
		}
	}
};