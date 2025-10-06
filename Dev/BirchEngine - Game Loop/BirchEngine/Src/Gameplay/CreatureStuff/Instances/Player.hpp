#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Game Management/Game.hpp"
//emitter stuff
#include <vector>
#include "Src/Gameplay/CreatureStuff/Condition.hpp"
#include "Src/Gameplay/AbilityStuff/Abilities.hpp"
#include "Src/Gameplay/UserInterface/HotbarEntity.hpp"
#include "Src/Gameplay/UserInterface/InventoryWindow.hpp"
static int playerCount = 0;
static int livingPlayerCount = 0;

class PlayerEntity : public CreatureEntity {
private:
public:
	PlayerEntity(Manager& entityManager) : CreatureEntity(entityManager, 10) {
		++playerCount;
		++livingPlayerCount;
		this->getComponent<ResourcesComponent>().getResource("Health") = Resource{ "Health",100.0f,1.0f };

		this->addComponent<SpatialComponent>(300.0f, 300.0f, 32, 32, 2.0f);
		this->addComponent<SpriteComponent>("slime", true, std::map<std::string, Animation>{ {"Idle", Animation{ 0, 6, 150 }}, { "Dead",Animation{2, 3, 100} }, { "Move",Animation{1, 6, 75} }});
		this->getComponent<SpriteComponent>().play("Idle", playType_loop);
		this->addComponent<MovementComponent>(Vector2D{ 0,0 }, 250.0f);
	
		this->addComponent<ColliderComponent>(HitboxShape::Rectangle,0u);
		this->addComponent<InputController>().loadCharacterControls();
		this->addGroup(EntityGroup::groupPlayers);

		//Game stuff
		//this->addComponent<ResourceBarComponent>(health.cur, SDL_Color{ 255,0,0,255 });
		//FIX: re-do health bar stuff

		Hotbar& hb = this->addComponent<DataComponent<Hotbar>>().data;
		hb.equipToEntity(*this);

		//problme originates here? hotbar is creted, hotbar input methods fail due to incorrect hotbar(?)
		// hotbar is created originally and has currentSlot=0. then when input is called, currentSlot=1480358904832048 or smth.
		entityManager.queueAddEntity<HotbarEntity>(&hb);
		entityManager.queueAddEntity<InventoryEntity>(&this->getComponent<DataComponent<Inventory>>().data);

		this->team = 1;
	}

	~PlayerEntity() {
		--playerCount;
		for (StatusEffect*& c : curEffects) {
			delete c;
		}
	}

	void blockPlayerInput() {
		this->getComponent<InputController>().blockInput(true);
	}

	//Entity* e: entity causing damage
	//FIX: delete this
	void takeDamage(Entity* e, float dmg) override {
		/*this->health -= dmg * damageTakenMultiplier;
		if (health <= 0) {
			this->die(false);
		}*/
	}

	//FIX: sort this out with the new event system.
	void die(bool TrueKill) {
		//for(auto& m : onDeathEffects) m();

		//if player MUST die OR the player was not healed/revived by an onDeathEffect
		//if (TrueKill || this->health <= 0) {
		//	blockPlayerInput();
		//	if(health>0) health = 0;
		//	this->health.regenRate = 0.0f;
		//	this->getComponent<SpriteComponent>().play("Dead",playType_playOnceAndFreeze); //make a "Dying" animation, and add a "play then freeze" option on top of loop/dont loop // bug: doesnt go to srcRect.x=0 cos its Dead has same id as Idle
		//	this->delGroup(Game::groupLabels::groupPlayers);
		//	this->addGroup(Game::groupLabels::groupItems);
		//}
	}
};