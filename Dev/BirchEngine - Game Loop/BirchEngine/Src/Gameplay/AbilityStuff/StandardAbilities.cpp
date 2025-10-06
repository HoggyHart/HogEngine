#include "StandardAbilities.hpp"
#include "Abilities.hpp"
#include "Src/Gameplay/Entities.hpp"

const Ability* StandardAbilities::shoot{
	new Ability{
		"Shoot",
		"This ability shoots to your cursor",
		new ActiveActivationMethod{SDL_BUTTON_LEFT},
		EffectMethod{[](Ability& a) {
			auto& actionVars = a.stats;
			Uint32& lastShot = *actionVars["lastAttackTime"]._Cast<Uint32>();
			Uint32& attackDelayMs = *actionVars["attackDelayMs"]._Cast<Uint32>();
			if (GameTime::totalTime - lastShot >= attackDelayMs) {
				Vector2D& bulletDirection = *actionVars["direction"]._Cast<Vector2D>();
				float& bulletSpeed = *actionVars["attackSpeed"]._Cast<float>();
				float& bulletScale = *actionVars["attackScale"]._Cast<float>();
				//creating bullet
				ProjectileEntity* pe = &a.restOfSet->abilitySetUser->entityManager.queueAddEntity<ProjectileEntity>(a.restOfSet->abilitySetUser->getComponent<SpatialComponent>().getCenter() - Vector2D(16 * bulletScale, 16 * bulletScale),
					bulletDirection,
					20000,
					bulletSpeed,
					"bullet",
					std::to_string(0), //should be team related i.e. &a.restOfSet->abilitySetUser->getTeamNumber()
					32,
					bulletScale,
					a.restOfSet->abilitySetUser);
				lastShot = GameTime::totalTime;
			}
		}},
		EffectStats{ {"lastAttackTime", Uint32(0)}, { "attackDelayMs", Uint32(150) }, { "direction",Vector2D(0, -1) }, { "attackSpeed", float(600.0f) }, { "attackScale", float(1) }}
	}
};

const Ability* StandardAbilities::shootAtCursor{
	new Ability{
		"Shoot",
		"This ability shoots to your cursor",
		new ActiveActivationMethod{SDL_BUTTON_RIGHT},
		EffectMethod{[](Ability& a) {
			Entity* abilityUser = a.restOfSet->abilitySetUser;
			auto& actionVars = a.stats;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);
			x += Game::camera.x;
			y += Game::camera.y;
			x = static_cast<int>(x / Game::camera.zoomAmount);
			y = static_cast<int>(y / Game::camera.zoomAmount);
			actionVars["direction"] = abilityUser->getComponent<SpatialComponent>().getCenter().direction(Vector2D(static_cast<float>(x), static_cast<float>(y)));
			Uint32& lastShot = *actionVars["lastAttackTime"]._Cast<Uint32>();
			Uint32& attackDelayMs = *actionVars["attackDelayMs"]._Cast<Uint32>();
			if (GameTime::totalTime - lastShot >= attackDelayMs) {
				Vector2D& bulletDirection = *actionVars["direction"]._Cast<Vector2D>();
				float& bulletSpeed = *actionVars["attackSpeed"]._Cast<float>();
				float& bulletScale = *actionVars["attackScale"]._Cast<float>();
				//creating bullet
				ProjectileEntity* pe = &abilityUser->entityManager.queueAddEntity<ProjectileEntity>(a.restOfSet->abilitySetUser->getComponent<SpatialComponent>().getCenter() - Vector2D(16 * bulletScale, 16 * bulletScale),
					bulletDirection,
					20000,
					bulletSpeed,
					"bullet",
					std::to_string(1), //should be team related i.e. &abilityUser->getTeamNumber()
					32,
					bulletScale,
					abilityUser);
				pe->addComponent<DataComponent<AttackInfo>>(1u, DamageStats{ dmgPhysical,dmgFlat,Stat{"Damage",1.0f}});

				lastShot = GameTime::totalTime;
			}
		}},
		EffectStats{ {"lastAttackTime", Uint32(0)}, { "attackDelayMs", Uint32(150) }, { "direction",Vector2D(0, -1) }, { "attackSpeed", float(600.0f) }, { "attackScale", float(0.5) }}
	}
};

const Ability* StandardAbilities::swingBat{
	new Ability{
		"Swing Bat",
		"Swing your bat",
		new ActiveActivationMethod{SDL_BUTTON_LEFT},
		EffectMethod{[](Ability& a) {
			Entity* abilityUser = a.restOfSet->abilitySetUser;
			auto& actionVars = a.stats;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);
			x += Game::camera.x;
			y += Game::camera.y;
			x = static_cast<int>(x / Game::camera.zoomAmount);
			y = static_cast<int>(y / Game::camera.zoomAmount);
			actionVars["direction"] = abilityUser->getComponent<SpatialComponent>().getCenter().direction(Vector2D(static_cast<float>(x), static_cast<float>(y)));
			Uint32& lastAttackTime = *actionVars["lastAttackTime"]._Cast<Uint32>();
			Uint32& attackDelay = *actionVars["attackDelayMs"]._Cast<Uint32>();
			if (GameTime::totalTime - lastAttackTime >= attackDelay) {
				Vector2D& attackDirection = *actionVars["direction"]._Cast<Vector2D>();
				float& slashSpeed = *actionVars["attackSpeed"]._Cast<float>(); //amount of full rotations/second  1 means 360* a second, etc.
				//PROBLEM: IF SET TO SLASH IN 360 DEGREES EVERY TICK, WILL COMPLETELY SKIP CIRCLE AND JUST END UP BACK WHERE IT STARTED EVERY TICK.
				//SOLUTIONS : 1. PATH COLLISION - NO CLUE HOW TO DO;
				//			  2. STRETCH HITBOX FROM START ROT TO END ROT EACH TICK, SO EACH TICK ENTITY WOULD BE SURROUNDED BY A CIRCULAR HITBOX IN CASE OF 100 SPEED
				float& areaScale = *actionVars["attackScale"]._Cast<float>();
				float& slashAngle = *actionVars["slashAngle"]._Cast<float>();

				std::pair<std::string, Animation> sprite{ "baseballBat",Animation{0,1,0} };

				double maxRotation = -0.5 * M_PI;
				AttackInfo atkinfo{ -1u, DamageStats{dmgPhysical,dmgFlat,Stat{"Damage",5.0f}} };
				atkinfo.setHitDelay(100);

				abilityUser->entityManager.queueAddEntity<SweepingEntity>(
					abilityUser,
					attackDirection.rotate(std::max(-slashAngle / 2.0,maxRotation), nullptr).toAngle(),
					slashAngle,
					abilityUser->getComponent<SpatialComponent>().width * abilityUser->getComponent<SpatialComponent>().scale,
					slashSpeed,
					AttackInfo{ -1u, DamageStats{dmgPhysical,dmgFlat,Stat{"Damage",5.0f}} },
					&sprite, nullptr);
				lastAttackTime = GameTime::totalTime;
			}
		}},
		EffectStats{{"lastAttackTime", Uint32(0)}, { "attackDelayMs", Uint32(500) }, { "direction",Vector2D(Vector2D(0, -1)) }, { "attackSpeed", float(2.0f) }, { "attackScale", float(0.5f) }, { "slashAngle", float(0.5 * M_PI) }}
	}
};

const Ability* StandardAbilities::onKillTest{
	new Ability{
		"Test",
		"Test",
		new PassiveActivationMethod{{WHE_EntityDeath}},
		EffectMethod{[](Ability& a) {
			std::cout << "KILLED !\n";
		}},
		EffectStats{ }
	}
};

const Ability* StandardAbilities::destructionWave{
	new Ability{
		"DESTROY",
		"This ability shoots to your cursor",
		new ActiveActivationMethod{SDL_BUTTON_LEFT},
		EffectMethod{[](Ability& a) {
			Entity* abilityUser = a.restOfSet->abilitySetUser;
			auto& actionVars = a.stats;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);
			x += Game::camera.x;
			y += Game::camera.y;
			x = static_cast<int>(x / Game::camera.zoomAmount);
			y = static_cast<int>(y / Game::camera.zoomAmount);
			actionVars["direction"] = abilityUser->getComponent<SpatialComponent>().getCenter().direction(Vector2D(static_cast<float>(x), static_cast<float>(y)));
			Uint32& lastShot = *actionVars["lastAttackTime"]._Cast<Uint32>();
			Uint32& attackDelayMs = *actionVars["attackDelayMs"]._Cast<Uint32>();
			if (GameTime::totalTime - lastShot >= attackDelayMs) {
				Vector2D& bulletDirection = *actionVars["direction"]._Cast<Vector2D>();
				float& bulletSpeed = *actionVars["attackSpeed"]._Cast<float>();
				float& bulletScale = *actionVars["attackScale"]._Cast<float>();
				//creating bullet

				int bulletsize = 1000;

				ProjectileEntity* pe = &abilityUser->entityManager.queueAddEntity<ProjectileEntity>(a.restOfSet->abilitySetUser->getComponent<SpatialComponent>().getCenter() - Vector2D(bulletsize / 2.0f * bulletScale, bulletsize / 2.0f * bulletScale),
					bulletDirection,
					20000,
					bulletSpeed,
					"bullet",
					std::to_string(1), //should be team related i.e. &abilityUser->getTeamNumber()
					bulletsize,
					bulletScale,
					abilityUser);
				//pe->getComponent<ColliderComponent>().setCallback(3);
				pe->addGroup(EntityGroup::groupGlobalColliders);
				lastShot = GameTime::totalTime;
			}
		}},
		EffectStats{ {"lastAttackTime", Uint32(0)}, { "attackDelayMs", Uint32(150) }, { "direction",Vector2D(0, -1) }, { "attackSpeed", float(600.0f) }, { "attackScale", float(1) }}
	}
};