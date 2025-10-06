#include "CollisionMethods.hpp"
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/Entities.hpp"

namespace CollisionMethods {
	const CCM defaultMethod = [](Entity* e, Entity* e2) {return false;//std::cout << "Default collision method!\n"; return false; };
		};

	const CCM instantPushOut = [](Entity* thisE, Entity* thatE) {
		SpatialComponent& thisSC = thisE->getComponent<SpatialComponent>();
		SpatialComponent& thatSC = thatE->getComponent<SpatialComponent>();
		SDL_Rect me = thisSC.toRect();
		SDL_Rect yu = thatSC.toRect();

		//calculates whether object should be pushed horizontally or vertically out
		//true/false = horizontal/vertical
		bool h = abs(0.5f - (thatSC.position.x + thatSC.width * thatSC.scale / 2.0f - thisSC.position.x) / thisSC.width * thisSC.scale) > abs(0.5f - (thatSC.position.y + thatSC.height * thatSC.scale / 2.0f - thisSC.position.y) / thisSC.height * thisSC.scale); //fix FIX: i do not know why this comment says fix

		//records if object stretches further above the midpoint of this hitbox i.e. if it should be pushed upward or downward
		//true/false = above/below
		bool a = 2 * thatSC.position.y + thatSC.height * thatSC.scale - 2 * thisSC.position.y - thisSC.height * thisSC.scale < 0;

		//records whether object should pushed left or right of this object
		//true/false = left/right
		bool l = 2 * thatSC.position.x + thatSC.width * thatSC.scale - 2 * thisSC.position.x - thisSC.width * thisSC.scale < 0;

		if (h) {
			if (l) thatSC.position.x = thisSC.position.x - thatSC.width * thatSC.scale - 1;
			else   thatSC.position.x = thisSC.position.x + thisSC.width * thisSC.scale + 1;
		}
		else {
			if (a) thatSC.position.y = thisSC.position.y - thatSC.height * thatSC.scale - 1;
			else   thatSC.position.y = thisSC.position.y + thisSC.height * thisSC.scale + 1;
		}

		return true;
		};

	const CCM attackHit = [](Entity* me, Entity* e2) {
		//NOTE: I considered adding support for AttackInfo* or AttackInfo& but i think it's too risky the way i imagined it.
		//		I was going to add it for proxy attacks and shared stats (i.e. if a circular slash would go over 360* it would spawn a second slash with the same stats and use AttackInfo* to use the same entityImmunityTracker
		//		BUT it runs the risk of if the first entity is destroyed the one with a pointer or reference would then be pointing or referencing an obejct which no longer exists
		//			- this could be mitigated with the use of a shared_ptr i think
		AttackInfo& stats = me->getComponent<DataComponent<AttackInfo>>().data;
		if (!me->hasComponent<DataComponent<EnemiesHit>>()) {
			me->addComponent<DataComponent<EnemiesHit>>();
		}
		//detect if entity has been hit yet, if not -> add to tracker and do effect inflicted by attack
		// if HAS been hit -> do nothing
		EnemiesHit& alreadyHit = me->getComponent<DataComponent<EnemiesHit>>().data;
		//POTFIX: maybe double-check the DamageEvent tracking here. I think all it is is timestamps and involved entities here so it'll be fine BUT if other stuff then note that the event may become deprecated by an ability and be innacurate
		//for each entity hit so far, if this is a previously hit entity and it has been long enough, hit again, then end callback
		for (DamageEvent& p : alreadyHit) {
			if (p.damagedEntity == e2) {
				//if still immune, quit callback
				if (GameTime::totalTime - p.timestamp < stats.getHitDelay()) return true;

				//if no longer immune, update immunity period start and do damage
				p.timestamp = GameTime::totalTime;
				static_cast<CreatureEntity*>(e2)->takeDamage(me, 1);
				return true;
			}
		}

		//if this entity hasnt been hit, add it to the tracker
		static_cast<CreatureEntity*>(e2)->takeDamage(me, 1);

		//FIX: if 1 instance of damage does flat AND %maxhp damage it will be split into two damageEvents.
		// FIND A WAY TO HAVE THEM CONSIDERED AS THE SAME INSTANCE OF DAMAGE. rough idea is commented in the loop marked IDEA
		for (auto& d : stats.damages) {
			WHE_Event& newEvent = WHE_QueueEvent(WHE_DamageEvent);
			new (&newEvent.event.damage) DamageEvent{ e2,me,d };
			//de.event.damage.damagedEntity = e2;
			//de.event.damage.damagerEntity = me;
			//de.event.damage.carryOverEvent = false;//IDEA: false on first, true on rest?
			////IDEA: de.event.damage.instanceID = instanceID;
			//de.event.damage.timestamp=Game::gameTimePassedMs();
			//de.event.damage.originalDamage = d;

			//FIX: regarding previous msg about deprecation, maye this should just store the timestamps and enemies hit

			alreadyHit.push_back(newEvent.event.damage);
		}

		stats.collisionsLeft -= 1;

		if (stats.collisionsLeft <= 0) {
			me->destroy();
			return false;
		}
		return true;
		};

	const CCM attackHitR = [](Entity* e1, Entity* e2) {
		//if attack has no info, this attack has not been initialised properly and will be treated as if it never collided
		if (!e1->hasComponent<DataComponent<AttackInfo>>()) throw;
		AttackInfo& atkInfo = e1->getComponent <DataComponent<AttackInfo>>().data;

		if (atkInfo.collisionsLeft <= 0) return false; //note there is no <0 here because it is unsigned

		//tracks whether this entity has been already or not
		DamageEvent* trackedHit{ nullptr };
		//if there is a delay, check if this target has been hit and cannot be hit again, prematurely ending the collision
		if (atkInfo.getHitDelay() > 0) {
			for (DamageEvent& p : atkInfo.enemiesHit) {
				if (p.damagedEntity == e2) {
					if (GameTime::totalTime - p.timestamp < atkInfo.getHitDelay())	return true;
					else {
						trackedHit = &p;
						break;
					}
				}
			}
		}

		if (trackedHit) trackedHit->timestamp = GameTime::totalTime;

		//queue damage events for each type of damage and handle start / addition to damage tracker
		for (auto& d : atkInfo.damages) {
			WHE_Event& newEvent = WHE_QueueEvent(WHE_DamageEvent);
			new (&newEvent.event.damage) DamageEvent{ e2,e1,d };
			//FIX: JUST MERGE ALL THE DAMAGES INTO ONE EVENT!!!!! for if
			//		REQUIRES CHANGING DAMAGEEVENT TO STORE MULTIPLE DAMAGES SO THAT recalulcateRawDamage() can work with it

			//FIX: STORE EVENTTRACKERS (or alternative) ONCE IMPLEMENTED TO NEGATE PROBLEMS CAUSED BY EVENT DEPRECATION

			//if no tracked hit, add this damage event to the tracker
			//this is for keeping track of the most recent time this entity was hit and all the damage it has taken as a result
			if (!trackedHit) {
				atkInfo.enemiesHit.push_back(newEvent.event.damage);
				trackedHit = &atkInfo.enemiesHit.back();
			}
			else {
				//FIX: update this when multiple damage types in 1 instance is sorted out
				//	it needs to add the whole vector of damage types
				trackedHit->originalDamages.push_back(newEvent.event.damage.originalDamage);
			}
		}

		//then apply status effects
		//FIX: figure out what to do with instant effects like knockback
		for (auto& s : atkInfo.effectsCaused) {
			//FIX: apply status effects from attack

			//im not 100% sure i like this, status effects might be an eventlistener reactive thing?
			//i think it depends though, like a poison tipped arrow is SURELY on the status effects caused bit for the arrow atk info,
			//	but on the other hand a buff that says "all damage you deal inflicts wither for 2 secs" is surely an event thing
			//it could be a passive ability of the ability/item ? needs more consideration
		}

		atkInfo.collisionsLeft -= 1; //could be moved into if statement,
		if (atkInfo.collisionsLeft > 0) return true;
		e1->destroy();
		return false;
		};

	const CCM destroyE2 = [](Entity* e1, Entity* e2) {
		//if (e2->getComponent<ColliderComponent>().tag != e1->getComponent<ColliderComponent>().tag) e2->destroy();
		e2->destroy();
		return true; };
}

using namespace CollisionMethods;
std::map<unsigned int, CollisionCallbackMethod> collisionCallbacks{
	//standard. basically the error case callback
	{0u, defaultMethod},

	//boundary keeping collision method
{ 1u,instantPushOut },

//deal damage
{ 2u, attackHitR },

{ 3u, destroyE2 }
};