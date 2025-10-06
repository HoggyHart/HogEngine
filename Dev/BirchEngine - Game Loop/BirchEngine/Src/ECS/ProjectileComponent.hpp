#pragma once

#include "Src/ECS/ECS.hpp"
#include "components.hpp"

class ProjectileComponent : public EComponent {
public:
	std::string tag;
	SpatialComponent* entitySpace = nullptr;
	MovementComponent* entityMovement = nullptr;
	Vector2D velocity;
	float range = 0;
	float baseSpeed = 0;
	float distance = 0;
	Entity* src = nullptr;

	ProjectileComponent(Entity& e, float range, float baseSpeed, Vector2D velocity, std::string tag, Entity* src) : EComponent(e) {
		this->tag = tag;
		this->range = range;
		this->baseSpeed = baseSpeed;
		this->velocity = velocity;
		this->src = src;

		if (this->getOwner()->hasComponent<SpatialComponent>())  entitySpace = &this->getOwner()->getComponent<SpatialComponent>();
		else std::cout << "CREATING PROJECTILE WITH NO POSITION\n"; //warning, but projectile component can be used like a timer // shgould probably be repalced by LifetimeComponent or smth
		if (this->getOwner()->hasComponent<MovementComponent>()) entityMovement = &this->getOwner()->getComponent<MovementComponent>();
		else entityMovement = &this->getOwner()->addComponent<MovementComponent>();
		entityMovement->velocity = this->velocity;
	}
	ProjectileComponent(Entity& e, ProjectileComponent& pc) : EComponent(e),
		tag(pc.tag),
		entitySpace(&e.getComponent<SpatialComponent>()),
		entityMovement(&e.getComponent<MovementComponent>()),
		velocity(pc.velocity),
		range(pc.range),
		baseSpeed(pc.baseSpeed),
		distance(pc.distance),
		src(pc.src) {
	}

	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<ProjectileComponent>(*this);
	}
	~ProjectileComponent() {};

	void fixedUpdate(bool EOI) override {
		if (distance >= range) {
			this->getOwner()->destroy();
		}

		else if (entitySpace) {
			Vector2D spCenter = entitySpace->getCenter();
			if (spCenter.x > Game::camera.x + Game::camera.w * 2.0f // if beyond right side( left side of screen + screen width  ) (i.e. out of view)
				|| spCenter.x < Game::camera.x - Game::camera.w * 1.5f/*or off the left side of t he screen*/
				|| spCenter.y < Game::camera.y - Game::camera.h * 1.5f
				|| spCenter.y > Game::camera.y + Game::camera.h * 2.0f) this->getOwner()->destroy();
		}
		distance += baseSpeed;
	}
	std::string getTag() {
		return tag;
	}
	Entity* getSource() {
		return src;
	}
	//private:
};