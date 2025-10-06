#pragma once
#include "Src/ECS/components.hpp"
//typedef struct ProjectileInfo {
//	Vector2D pos;
//	Vector2D velocity;
//	int range;
//	int speed; std::string spriteID;
//	std::string tag; int projSize; float projScale;
//};

typedef std::function<void()> ProjectileBehaviourMethod;

class ProjectileEntity : public Entity {
public:
	ProjectileEntity(Manager& man, Vector2D pos, Vector2D velocity, int range, float speed, std::string spriteID, std::string tag, int projSize, float projScale, Entity* src) : Entity(man) {
		this->addComponent<SpatialComponent>(pos.x, pos.y, projSize, projSize, projScale);
		this->addComponent<MovementComponent>(velocity, speed);
		this->addComponent<SpriteComponent>("bulletPNG");
		this->addComponent<ProjectileComponent>(static_cast<float>(range), speed / Game::TPS, velocity, tag, src);
		this->addComponent<ColliderComponent>(HitboxShape::Circle,2u);
		this->addGroup(EntityGroup::groupProjectiles);
	}

	void setProjectileBehaviour(ProjectileBehaviourMethod transitBehaviour, CollisionCallbackMethod collisionBehaviour) {
	}
};