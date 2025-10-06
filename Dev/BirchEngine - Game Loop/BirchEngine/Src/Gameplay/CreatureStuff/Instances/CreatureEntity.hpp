#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/CreatureStuff/Condition.hpp"
#include "Src/Gameplay/UserInterface/FallingText.hpp"
#include <map>
#include "Src/Gameplay/CreatureStuff/LimitedResource.hpp"
#include "Src/Gameplay/ItemStuff/Inventory.hpp"

class CreatureEntity : public Entity {
protected:
	int team = 0;
public:
	//std::map<std::string, Action*> actions;
	float damageTakenMultiplier = 1.0f;
	CreatureEntity(Manager& man, float maxHealth) : Entity(man) {
		this->addComponent<DataComponent<Inventory>>();
		ResourcesComponent& rc = this->addComponent<ResourcesComponent>();

		rc.addResource(rsrcHitPoints, { "Health", maxHealth, 0.0f });
	}
	std::vector<StatusEffect*> curEffects;

	void applyEffect(StatusEffect* e) {
		for (StatusEffect*& c : curEffects) {
			if (c->getName() == e->getName()) {
				if (c->getStacks() < c->maxStacks) {
					c->addStack();
					c->changeValue(e->change);
					delete e;
					return;
				}
				else {
					c->refreshStack();
					c->changeValue(e->change);
					delete e;
					return;
				}
			}
		}
		curEffects.push_back(e);
	}

	void fixedUpdate(bool EOI) override {
		//component updates
		for (auto& c : components) c->fixedUpdate(EOI);
		for (StatusEffect*& c : curEffects) c->fixedUpdate();

		//deleting expired conditions
		curEffects.erase(std::remove_if(curEffects.begin(), curEffects.end(), [](StatusEffect*& c) {
			if (c->stacks == 0) {
				delete c;
				return true;
			}
			return false;
			}), curEffects.end());
	}

	//FIX: DELETE THIS
	//SHOULD BE MOVED INTO THE EVENT HANDLING METHOD FOR DAMAGEEVENTS
	virtual void takeDamage(Entity* e, float dmg) {
		//this->health -= dmg*damageTakenMultiplier;

		SDL_Color d = { 255,255,255,255 };

		entityManager.queueAddEntity<FallingTextEntity>(static_cast<int>(this->getComponent<SpatialComponent>().position.x - 10 + rand() % (static_cast<int>(this->getComponent<SpatialComponent>().width * this->getComponent<SpatialComponent>().scale + 10))),
			static_cast<int>(this->getComponent<SpatialComponent>().position.y + this->getComponent<SpatialComponent>().height),
			"-" + std::to_string(static_cast<int>(dmg)),
			"Consolasx24",
			d,
			500);
	}

	int getTeamNumber() const {
		return team;
	}
};