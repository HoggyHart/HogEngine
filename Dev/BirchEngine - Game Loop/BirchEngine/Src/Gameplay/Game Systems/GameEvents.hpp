#pragma once
#include <queue>
#include "Src/Gameplay/AbilityStuff/AttackStats.hpp"
#include <iostream>
class Entity;
enum WHE_EventType {
	WHE_NoEvent,

	WHE_EntitySpawnEvent,
	WHE_EntityDeath,

	WHE_DamageEvent,
	WHE_DamageCarryOver,
	WHE_EntityHealed,

	WHE_EntityExitCombat,
	WHE_EntityEnterCombat,

	WHE_EntityMove
};

struct SpawnEvent {
	WHE_EventType type;

	Entity* spawned;
};
struct DamageEvent {
	WHE_EventType type;

	Entity* damagedEntity;
	Entity* damagerEntity;

	Uint32 timestamp; //game time

	std::vector<DamageStats> originalDamages{};
	DamageStats originalDamage;

	DamageEvent(Entity* victim, Entity* damager, DamageStats damage);
	~DamageEvent() {};
	//NOTE: carry overs are for if an entity has 2 hp bars that treat damage differently, i.e. a shield which takes it in full and health which takes reduced damage from a passive ability.
	//	damage must be marked as CarryOver to indicate that it is not a separate damage instance so that it doesn't proc onHit or whenHit effects more than once.
	//	carryOver events contain info on the original DamageEvent and info on the remaining damage.

	//	//ignores all info on damage mods POTFIX: i dont know about this man...
	//	//just carries the damage value and the type (i.e. physical/%maxhp etc.)
	bool carryOverEvent;
	DamageStats remainingDamage;
};
struct DeathEvent {
	WHE_EventType type;

	Entity* victim;
	Entity* killer;

	DamageEvent killingBlowInfo;

	DeathEvent(Entity* v, Entity* k, DamageEvent di) : type(WHE_EntityDeath), victim(v), killer(k), killingBlowInfo(di) {}

	~DeathEvent() {};
};
struct HealEvent {
	WHE_EventType type;

	Entity* healed;
	float amountHealed;
};
struct MoveEvent {
	WHE_EventType type;

	Entity* e;

	//Vector2D before;
	//Vector2D after;
	float distance;
	MoveEvent(Entity* m, float d) : type(WHE_EntityMove), e(m), distance(d) {}
	~MoveEvent() {};
};
struct WHE_Event;
extern std::queue<WHE_Event> gameEventQueue;

//FIX: implement event ID system to prevent response to same event multiple times in case of delays
//	consider modifiers in this order:
//		1. damage mod -= 0.5: simple
//		2. negate damage: deprecate and delay
// extra3. cancel negate: deprecate and delay
//	 then consider that the negate gets cancelled out:
//		Step 1: damage mod 1 -> 0.5
//		Step 2: negate: deprecate and delay
//		Step 3: event comes back to the front of the queue
//		Step 4: damage mod 0.5 -> 0.0 !!! ALREADY PROBLEM HERE. WITH NO NEGATION CANCEL THIS GOES THROUGH
//		Step 5: cancel negate: deprecate and delay
//		Step 6: back to front
//		Step 7: damage mod 0.0 -> -0.5 !!! NOW CREATURE IS HEALING FROM ATTACK!?!?!?
//		damage is down 150% instead of just 50%
//
struct WHE_Event {
private:
	bool deprecated{ false };
	std::vector<WHE_Event*> delayedEvents; //not sure when an event would d&d more than one other event but vector just in case.
public:
	WHE_EventType type; //shared with all Event objects

	union Event {
		WHE_EventType type;
		SpawnEvent spawn;
		DeathEvent death;
		DamageEvent damage;
		HealEvent heal;
		MoveEvent move;

		Event() :type(WHE_NoEvent) {}
		Event(const Event& e) {
			*this = e;
		}
		Event& operator=(const Event& e) {
			type = e.type;
			switch (type) {
			case WHE_NoEvent:
				break;
			case WHE_DamageEvent:
				new (&damage) DamageEvent{ e.damage };
				break;
			case WHE_EntityDeath:
				new (&death) DeathEvent{ e.death };
				break;
			case WHE_EntitySpawnEvent:
				spawn = e.spawn;
				break;
			case WHE_EntityMove:
				new (&move) MoveEvent{ e.move };
				break;
			default:
				throw;
			}
		}
		~Event() {
			switch (type) {
			case WHE_NoEvent:
				break;
			case WHE_EntityDeath:
				death.~DeathEvent();
				break;
			case WHE_EntityMove:
				move.~MoveEvent();
				break;
			default:
				std::cout << "MEMORY LEAK AT WHE_EVENT DESTRUCTOR!\n";
			}
		}
	} event{};

	// delay() is a more appropriate name but i like the alliteration
	WHE_Event* deprecateAndDelay() {
		//delay copy
		gameEventQueue.push(*this);
		//grab copy's address for returning
		WHE_Event* newThis = &gameEventQueue.back(); //POTFIX: i had issues with pointers to things in a vector when the vector was resized, be careful about queue behaving similarly

		//deprecate and repeat process for events delayed by this event
		deprecated = true;
		for (std::size_t i = 0; i < delayedEvents.size(); ++i) {
			delayedEvents[i] = delayedEvents[i]->deprecateAndDelay();
		}

		return newThis;
	}

	//no undeprecate() method because in theory nothing should ever be un-deprecated.
	void deprecate() {
		deprecated = true;
	}
	bool isDeprecated() const { return deprecated; }

	WHE_Event() : type(WHE_NoEvent) {};
	WHE_Event& operator=(const WHE_Event& ev) {
		deprecated = ev.deprecated;
		delayedEvents = ev.delayedEvents;
		type = ev.type;
		event = ev.event;
		return *this;
	}
	~WHE_Event() {
	}
};

//FIX: implement this for event tracking
//	current idea:
//		Tracker will hold a pointer to the event it is tracking(or ID when/if i implement that)
//		.deprecate() and .delay() will either:
//			A) look through that event's vector of trackers to either inform of the deprecation or pass the pointer of the delayed event
//			B) be checked after the modification loop to see the deprecation.
//					events will hold pointers to their delayed versions maybe. if not delayed it will be nullptr.
//					this info will be passed to the active eventtracker update loop just before the listeners
//					if an event isnt deprecated then the event tracker makes a copy and stops checking for the event
//
struct WHE_EventTracker {
	bool active{ true };

	union {
		WHE_Event* tracked;
		WHE_Event eventCopy;
	};

	WHE_EventTracker(WHE_Event* ptr) {
		new (&tracked) WHE_Event* { ptr };
	}
	WHE_EventTracker(WHE_Event ev) {
		new (&eventCopy) WHE_Event{ ev };
		active = false;
	}
};

//returns whether queue is still not clear
//1 = more events
//0 = clear
static inline bool WHE_PollEvent(WHE_Event* ptr) {
	//if no events, return with no event/false
	if (gameEventQueue.empty()) {
		*ptr = WHE_Event{};
		return false;
	}

	//FIX: to be implemented(/corrected?), refer to GameEventListener.hpp for more info
	/// if(ptr->deprecated == true){
	/// //if event is deprecated, skip to the next event
	///		return WHE_PollEvent(ptr);
	/// }

	//if valid
	*ptr = gameEventQueue.front();
	gameEventQueue.pop();
	return true;
}
static inline void WHE_ClearEvents() {
	gameEventQueue = std::queue<WHE_Event>();
}

static inline WHE_Event& WHE_QueueEvent(WHE_EventType type) {
	gameEventQueue.push(WHE_Event{});
	gameEventQueue.back().type = type;
	return gameEventQueue.back();
}

//FIX: i think declare EventController here (and also think of a more accurate name, EventFinaliser seems pretty good)
// //also note: eventcontroller will only be reached after event is confimred NOT deprecated and AFTER listeners, so all events that reach it should be
// //good to go
//so switch depending on event type, if its a request type i.e. damageEvent
// if type == damageEvent {
//		victim->takeDamage(&damager, damageInfo);
// }
//and so on
//
//
//

extern void finaliseEvent(WHE_Event& ev);