#include "Condition.hpp"
#include "Src/Gameplay/CreatureStuff/Instances/CreatureEntity.hpp"
bool InvincibilityStatusEffect::fixedUpdate()
{
	bool allTicking = true;
	std::vector<Uint32*> expired;
	for (Uint32& applyTime : applyTimes) {
		static_cast<CreatureEntity*>(applicant)->damageTakenMultiplier = 0.0f;
		if (GameTime::totalTime - applyTime > duration) {
			static_cast<CreatureEntity*>(applicant)->damageTakenMultiplier = 1.0f;
			stacks -= 1;
			expired.push_back(&applyTime);
			allTicking = false;
		}
	}
	for (Uint32*& p : expired) {
		applyTimes.erase(std::remove_if(applyTimes.begin(), applyTimes.end(), [&p](Uint32& i) {return &i == p; }), applyTimes.end());
	}
	return allTicking;
}

StatusEffect::StatusEffect(std::string conName, changeTypes t, float change, Uint32 duration, int maxStacks, Entity* applicant) : statusEffectName(conName), type(t), change(change), duration(duration), maxStacks(maxStacks), applicant(applicant), applyTimes({ GameTime::totalTime }) { stacks = 1; }

void StatusEffect::addStack()
{
	++stacks;
	applyTimes.push_back(GameTime::totalTime);
}

void StatusEffect::refreshStack()
{
	Uint32* earliestStack = &applyTimes[0];
	for (Uint32& at : applyTimes) {
		if (at < *earliestStack) {
			earliestStack = &at;
		}
	}
	*earliestStack = GameTime::totalTime;
}

bool SpeedStatusEffect::fixedUpdate()
{
	bool allTicking = true;
	std::vector<Uint32*> expired;
	for (Uint32& applyTime : applyTimes) {
		applicant->getComponent<MovementComponent>().speed.mods[type] += change;
		if (GameTime::totalTime - applyTime > duration) {
			applicant->getComponent<MovementComponent>().speed.mods[type] -= change;
			stacks -= 1;
			expired.push_back(&applyTime);
			allTicking = false;
		}
	}
	for (Uint32*& p : expired) {
		applyTimes.erase(std::remove_if(applyTimes.begin(), applyTimes.end(), [&p](Uint32& i) {return &i == p; }), applyTimes.end());
	}
	return allTicking;
}