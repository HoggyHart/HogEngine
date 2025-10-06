#pragma once
#include <vector>
#include "Src/Gameplay/CreatureStuff/StatsAndModifiers.hpp"
#include "Src/Gameplay/CreatureStuff/LimitedResource.hpp"
#include "Src/Gameplay/CreatureStuff/Condition.hpp"
typedef unsigned int Uint32;

enum DamageType { dmgPhysical, dmgMagical, dmgTrue };
//idk if "scale" is the best descriptor here
enum DamageScale { dmgFlat, dmgPrctMax, dmgPrctMissing, dmgPrctCurrent };

typedef std::pair<DamageScale, Stat> Damage;
//FIX: also include whether damage can be reduced or not
struct DamageStats {
	DamageType type{ dmgTrue };
	DamageScale scale{ dmgFlat };
	Stat amount{ "Damage", 0.0f };
	DamageStats() {}
	DamageStats(DamageType t, DamageScale scale, Stat damage) : type(t), scale(scale), amount(damage) {}

	float recalculateRawDamage(Resource health) {
		amount.recalculateTotal();
		switch (scale) {
		case dmgFlat:
			break;
		case dmgPrctMax:
			amount.value *= health.max;
			break;
		case dmgPrctCurrent:
			amount.value *= health.cur;
			break;
		case dmgPrctMissing:
			amount.value *= health.max - health.cur;
			break;
		}
		return amount.value;
	}
};

//store pointer to DamageEvents caused by hitting enemies
//NOTE: storing of damage event can be used for stacking dmg i.e.
//		each entity hit adds the damage they took to the projectiles damage
//		collision events -> 2 enemies hit -> enemy 1 takes X damage -> x dmg is mitigated by armour -> damage event stores X-x damage -> enemy 2 is hit -> takes X + X-x (from damageevent).
// POTFIX:
//		game event loop -> enemy 1 mitigates some more damage via passive Ability. enemy 2 has already taken the damage, should this have affected enemy 2's damage? how would that happen?
//
// MAYBEFIX: damage SHOULDN'T carry over until after all the collisions have been handled,
//	i.e. collisions -> both take X and Y damage respectively -> attack has listener that stores copies of the damageEvents AFTER they have been modified by dmg mitigation (listener must be at end of listeners to catch it last and before events expire)
//		on next collision damageEvents are stored at their final damage values for next hits
class DamageEvent;
typedef std::vector<DamageEvent> EnemiesHit;

struct AttackInfo {
private:
	//these two are privated because they are linked together. if hit delay is >0 trackEnemies HAS to be true
	Uint32 sameTargetHitDelay{ 0 };
	bool trackEnemiesHit{ true };
public:
	EnemiesHit enemiesHit{};

	Uint32 collisionsLeft = 1;

	std::vector<DamageStats> damages{ {dmgPhysical, dmgFlat, Stat{"Damage",1.0f} } };

	std::vector<StatusEffectID> effectsCaused{};

	AttackInfo() {};
	AttackInfo(Uint32 dura, DamageStats dmg) : collisionsLeft(dura), damages({ dmg }) {}

	//normal setDelay() method
	inline void setHitDelay(Uint32 delay) {
		sameTargetHitDelay = delay;
		if (delay == 0) trackEnemiesHit = true;
	}

	//setDelay() for passing 0 when yuou want
	inline void setHitDelay(Uint32 delay, bool trackEnemies) {
		sameTargetHitDelay = delay;
		if (delay > 0) trackEnemiesHit = true;
		if (delay == 0) trackEnemiesHit = trackEnemies;
	}
	inline Uint32 getHitDelay() const { return sameTargetHitDelay; }
};