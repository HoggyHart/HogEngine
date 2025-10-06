#pragma once
#include <map>
#include <functional>
#include "Src/ECS/ECS.hpp"
//takes 2 entities, the entity using the method and the entity collided with
//return value indicates whether ignoring the other entity's collision method
//	true: ignore other entity collision method
//	false: let other entity collide
typedef std::function<bool(Entity* E1, Entity* E2)> CollisionCallbackMethod;
typedef CollisionCallbackMethod CCM;

typedef unsigned int CollisionCallbackID;

namespace CollisionMethods {
	//just says "hey! a collision happened!"
	extern const CCM defaultMethod;

	//requires both entities have SpatialComponents
	extern const CCM instantPushOut;

	//entity gets given tracker for immunityPeriods if doesnt have it already
	extern const CCM attackHit;
	//entity gets given tracker for immunityPeriods if doesnt have it already
	extern const CCM attackHitR;

	//straight up destroys E2 unless it has the same tag as E1
	extern const CCM destroyE2;
};

extern std::map<CollisionCallbackID, CollisionCallbackMethod> collisionCallbacks;