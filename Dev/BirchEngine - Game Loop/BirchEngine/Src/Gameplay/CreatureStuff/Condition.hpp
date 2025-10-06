#pragma once
#include <string>
#include "Src/Gameplay/CreatureStuff/StatsAndModifiers.hpp"
#include "Src/ECS/ECS.hpp"
typedef unsigned int StatusEffectID;

class StatusEffect {
public:
	std::string statusEffectName;
	std::vector<Uint32> applyTimes;
	Uint32 duration; //in ms
	changeTypes type;
	Entity* applicant;
	int stacks;
	int maxStacks;
	float change;

	StatusEffect(std::string conName, changeTypes t, float change, Uint32 duration, int maxStacks, Entity* applicant);
	~StatusEffect() {}
	std::string getName() { return statusEffectName; }
	int getStacks() { return stacks; }

	virtual bool fixedUpdate() {
		return false;
	}

	void changeValue(float ch) { change = ch; }

	void addStack();

	void refreshStack();
};

class SpeedStatusEffect : public StatusEffect {
private:

public:

	SpeedStatusEffect(std::string conName, changeTypes type, float change, Uint32 dur, int maxStacks, Entity* afflicted) : StatusEffect(conName, type, change, dur, maxStacks, afflicted) {}

	bool fixedUpdate() override;
};

class InvincibilityStatusEffect : public StatusEffect {
private:

public:

	InvincibilityStatusEffect(std::string conName, changeTypes type, float change, Uint32 dur, int maxStacks, Entity* afflicted) : StatusEffect(conName, type, change, dur, maxStacks, afflicted) {}

	bool fixedUpdate() override;
};