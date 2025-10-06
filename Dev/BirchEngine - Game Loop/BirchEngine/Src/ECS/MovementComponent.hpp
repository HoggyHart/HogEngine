#pragma once
#include "Src/Game Management/GameTime.hpp"
#include "Src/ECS/ECS.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Gameplay/CreatureStuff/StatsAndModifiers.hpp"
#include "SpatialComponent.hpp"
#include <map>
#include <functional>

class MovementComponent : public EComponent {
public:
	Vector2D velocity;
	Stat speed{ "speed", 200.0f };

	Stat rotationalSpeed{ "rotationSpeed", 2 * M_PI };
	Vector2D rotationalVelocity;

	SpatialComponent* space{ nullptr };

	MovementComponent(Entity& e) : EComponent(e), space(&e.getComponent<SpatialComponent>()) {
		velocity.zero();
		rotationalVelocity.zero();
		this->getOwner()->addGroup(EntityGroup::groupInterpolatables);
	}
 	MovementComponent(Entity& e, Vector2D velocity, float baseSpeed) : EComponent(e), velocity(velocity), speed("speed", baseSpeed), space(&e.getComponent<SpatialComponent>()) {
		rotationalVelocity.zero();
		this->getOwner()->addGroup(EntityGroup::groupInterpolatables);
	};
	MovementComponent(Entity& e, MovementComponent& mc) : EComponent(e),
		space(&e.getComponent<SpatialComponent>()),
		velocity(mc.velocity),
		speed(mc.speed),
		rotationalSpeed(mc.rotationalSpeed),
		rotationalVelocity(mc.rotationalVelocity)

	{
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<MovementComponent>(*this);
	}
	~MovementComponent() {}

	void move(Uint32 timeMs) {
		float speedNormaliser = velocity.magnitude(); //note the velocity of the entity
		if (speedNormaliser != 0.0f) {
			//calculate speed of entity after modifiers
			float curSpeed = speed.value;
			curSpeed *= timeMs / 1000.0f;

			//velocity normalisation. speed is divided by vMag to slow it down if the magnitude is greater than the intended const value of 1 or speed it up if it somehow gets below 1.
			space->position.x += velocity.x * curSpeed / speedNormaliser;
			space->position.y += velocity.y * curSpeed / speedNormaliser;
			//side note: if the velocity mag ever reached <1, the player would likely always be moving regardless of input due to the +/-1s to velocity in inp-ut controller
		}
		if (rotationalVelocity.x != 0) {
			float rotSpeed = rotationalSpeed.value;
			rotSpeed *= timeMs / 1000.0f;

			space->rotation.add(rotationalVelocity.x * rotSpeed);
		}
	}

	void fixedUpdate(bool EOI) override {

		speed.recalculateTotal();
		rotationalSpeed.recalculateTotal();

		if (velocity.magnitude() >= 0) {
			//game event
			WHE_Event& ev = WHE_QueueEvent(WHE_EntityMove);
			(new (&ev.event.move) MoveEvent{ this->getOwner(),speed.recalculateTotal() * BASE_GAME_MS_PASSED_PER_TICK / 1000.0f });
			
			//actual move
			if (EOI) move(GameTime::deltaTime);
			else move(GameTime::fixedDeltaTime);
		}
		
		speed.resetMods();
		rotationalSpeed.resetMods();

	}
	void interUpdate() override {
		move(GameTime::deltaTime);
	}
};