#pragma once
#include "Src/ECS/components.hpp"
class SweepingEntity : public Entity {
public:

	Entity* source = nullptr;

	float arcSize = 0.0f; //determines amount of 360 degree slashes to do as part of this slash
	float arcRadius = 0.0f; //determines radius of slash arc
	float arcSpeed = 0.0f; //stores angle change in 1 second

	Uint32 linger{ 0 }; //determines ms that hitbox will remain for
	AttackInfo atkStats;

	typedef std::pair<std::string, Animation> entitySprite;
	typedef std::pair<std::string, Animation> slashTrail;

	//slashAngle: angle the arc will cover (radians)
	//slashReach: length of the arc from the point to the edge (pixels)
	//arcSpeed: percentage of full arc rotation completed per second (1.0 being 1 full rotation a second)
	SweepingEntity(Manager& entityManager,
		Entity* src,
		Angle startAngle,
		float slashAngle,
		float slashReach,
		float rotationsPerSecond,
		AttackInfo atkInfo,
		entitySprite* slashSprite,
		slashTrail* trail)
		:
		Entity(entityManager),
		source(src),
		arcSize(slashAngle),
		arcRadius(slashReach),
		arcSpeed(rotationsPerSecond * 2.0f * static_cast<float>(M_PI)),
		atkStats(atkInfo)
	{
		//add to world, bind to source entity, and enforce link to set in position
		SpatialComponent& sc = this->addComponent<SpatialComponent>(0.0f, 0.0f, static_cast<int>(slashReach * 2), static_cast<int>(slashReach * 2), 1.0f);
		SpatialComponent& srcSC = src->getComponent<SpatialComponent>();
		LinkComponent& l = src->getComponent<SpatialComponent>().addLink(sc, std::pair<Vector2D, Vector2D>{{(srcSC.width* srcSC.scale) / 2.0f, (srcSC.height* srcSC.scale) / 2.0f}, { sc.width * sc.scale / 2.0f,sc.height * sc.scale / 2.0f }});
		l.setLinkDistance(0.0f);
		l.fixedUpdate(false);

		//add tracker component for tracking the entities and when they were hit, used for preventing repeat damage too fast
		//remove entities from tracker when they are able to bit hit again

		MovementComponent& mc = this->addComponent<MovementComponent>();
		//range = slash angle (i.e. full circle slash means range = 2*M_PI)
		//speed = slashes/s multiplied by how many seconds pass each update step (i.e. 1 slash/s at 20 tps == 2*M_PI * 0.05
		this->addComponent<ProjectileComponent>(slashAngle, arcSpeed * GameTime::fixedDeltaTime/1000.0f, Vector2D{ 0,0 }, "1", src);

		//add hitbox
		ColliderComponent& cc = this->addComponent<ColliderComponent>(HitboxShape::Rectangle, 2);
		//add callback method that will add an enemy and the current time to the tracker if they havent been hit yet
		this->addComponent<DataComponent<AttackInfo>>(atkInfo);

//		//add sweeping motion.
//		//	currently the NEGATIVE part of the rotation is due to moving counter-clockwise. if wanted to move the other way, must be refactored to take that into account
//		this->addComponent<CustomFunc>([this, &srcSC]() {
//			ColliderComponent& cc = this->getComponent<ColliderComponent>();
//			ProjectileComponent& pc = this->getComponent<ProjectileComponent>();
//			float moveFactor = pc.range - pc.distance < pc.baseSpeed ? -((pc.range - pc.distance) / pc.baseSpeed) : -1; // determine if necessary to limit the arc to the end of the slash
//
//			if (this->arcSpeed < 2 * M_PI || moveFactor * this->arcSpeed > -2 * M_PI) {
//				static_cast<CollisionAreaArc*>(cc.hitbox)->th2 = static_cast<CollisionAreaArc*>(cc.hitbox)->th1;
//				static_cast<CollisionAreaArc*>(cc.hitbox)->th1.add(moveFactor * this->arcSpeed * GameTime::fixedDeltaTime/1000.0f);
//			}
//
//			else {
//				//else: a full slash was done -> add an extra slashEntity to act as each full circle slash that occurred in this single tick
//				//original slashEntity continues from where the slash finished off on that tick
//#pragma region fullSlashEntity
//				for (int i = 0; i < static_cast<int>(this->arcSpeed * GameTime::fixedDeltaTime/1000.0f / (2 * M_PI)); ++i) {
//					Entity* fullSlash = &this->entityManager.queueAddEntity<Entity>();
//
//					SpatialComponent& tf = this->getComponent<SpatialComponent>();
//					fullSlash->addComponent<SpatialComponent>(0.0f, 0.0f, tf.width, tf.height, tf.scale);
//					LinkComponent& l = srcSC.addLink(fullSlash->getComponent<SpatialComponent>());
//					l.setLinkDistance(0.0f);
//					l.fixedUpdate(false);
//
//					fullSlash->addComponent<ColliderComponent>("slash", HitboxShape::Circle);
//
//					fullSlash->addComponent<DataComponent<AttackInfo>>(this->getComponent<DataComponent<AttackInfo>>().data);
//					fullSlash->getComponent<ColliderComponent>().setCallback(2);
//
//					fullSlash->addComponent<ProjectileComponent>(pc.range, pc.baseSpeed, pc.velocity, pc.tag, pc.src);
//					fullSlash->getComponent<ProjectileComponent>().distance = fullSlash->getComponent<ProjectileComponent>().range + 1; // set range to delete this projectile NEXT tick.
//					fullSlash->addGroup(EntityGroup::groupProjectiles);
//				}
//#pragma endregion
//				static_cast<CollisionAreaArc*>(cc.hitbox)->th2 = static_cast<CollisionAreaArc*>(cc.hitbox)->th1;
//				static_cast<CollisionAreaArc*>(cc.hitbox)->th1.add(moveFactor * this->arcSpeed * GameTime::fixedDeltaTime/1000.0f);
//			}
//			});

		//adding visual stuff
		if (slashSprite) {
			Entity* sprite = &entityManager.queueAddEntity<Entity>();
			this->addDependant(sprite); //the sprite should be destroyed if the slashing entity is destroyed

			float spriteScale = slashReach / 32.0f;
			SpatialComponent& ssc = sprite->addComponent<SpatialComponent>(0.0f, 0.0f, 32, 32, spriteScale);
			ssc.rotation.add(0.5 * M_PI + startAngle);

			LinkComponent& l = this->getComponent<SpatialComponent>().addLink(ssc);
			l.setLinkDistance(16 * spriteScale);
			l.setLinkAngle(startAngle);
			l.setLinkAngleChangePS(-arcSpeed, true);
			l.fixedUpdate(false);

			sprite->addComponent<SpriteComponent>(slashSprite->first, false, std::map<std::string, Animation>{ {"Idle", slashSprite->second} });

			sprite->addComponent<TrailComponent>(75, 0);
			sprite->addGroup(EntityGroup::groupWeaponEffects);
		}

		this->addGroup(EntityGroup::groupProjectiles);
		this->fixedUpdate(false);
	}

};