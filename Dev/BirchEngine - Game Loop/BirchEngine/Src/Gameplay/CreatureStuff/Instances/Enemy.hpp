#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/CreatureStuff/Instances/CreatureEntity.hpp"
class EnemyEntity : public CreatureEntity {
public:
	EnemyEntity(Manager& entityManager, Vector2D pos) : CreatureEntity(entityManager, 2) {
		this->addComponent<SpatialComponent>(pos.x, pos.y, 32, 32, 2.0f);
		
		this->addComponent<SpriteComponent>("greenGuy", true, std::map<std::string, Animation>{{"Idle", Animation{ 0,8,200 } }});
		this->getComponent<SpriteComponent>().play("Idle", playType_loop);
		this->getComponent<SpriteComponent>().spriteFlip = SDL_FLIP_HORIZONTAL;
		this->addComponent<MovementComponent>(Vector2D{ 0,0 }, 50.0f);
		this->addComponent<ColliderComponent>(HitboxShape::Circle,0u);
		//FIX: DEPRECATED. TURN INTO ENTITY: this->addComponent<ResourceBarComponent>(health.cur, SDL_Color{ 255,0,0,255 });
		this->addGroup(EntityGroup::groupEnemies);
		//	this->actions.emplace("Primary", new Action(*Game::actionManager->getObject("shootInDirection")));
		this->team = 2;
	}
	~EnemyEntity(){
		std::cout << "Enemy Entity destructor\n";
	}

	void setAI(Behaviour* b) {
		this->addComponent<BehaviourComponent>(b);
	}
};