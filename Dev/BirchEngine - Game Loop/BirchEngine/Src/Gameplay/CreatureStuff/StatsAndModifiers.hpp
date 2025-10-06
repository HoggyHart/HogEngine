#pragma once
#include <string>
#include <map>

enum changeTypes {
	statModBaseFlat = 'f',
	statModBaseMultiplicative = 'm',
	statModFlat = 'F',
	statModMultiplicative = 'M'
};

struct StatModifiers {
	std::map<changeTypes, float> statMods{ {statModBaseFlat,0.0f},{statModBaseMultiplicative,1.0f},{statModFlat,0.0f},{statModMultiplicative,1.0f} };

	StatModifiers() {};

	void reset() {
		statMods[statModBaseFlat] = 0.0f;
		statMods[statModBaseMultiplicative] = 1.0f;
		statMods[statModFlat] = 0.0f;
		statMods[statModMultiplicative] = 1.0f;
	}

	float& operator[](changeTypes c) {
		return statMods[c];
	}

	float applyToVal(float val) {
		val += statMods[statModBaseFlat];
		val *= statMods[statModBaseMultiplicative];
		val += statMods[statModFlat];
		val *= statMods[statModMultiplicative];
		return val;
	}
};

struct Stat {
	std::string statName{ "" };
	float baseValue{ 0.0f };
	StatModifiers mods{};
	//FIX: recalculate value every game tick so not needed to recalculate? not sure where to do that though. Dare I say that Stat should be a component? noooo nonono dont like that
	float value{ 0.0f };

	Stat() : statName("Unknown Stat"), baseValue(0) {};
	Stat(std::string name, float v) : statName(name), baseValue(v) {};
	Stat(std::string name, int v) : statName(name), baseValue(static_cast<float>(v)) {};
	Stat(std::string name, double v) : statName(name), baseValue(static_cast<float>(v)) {};

	float afterMods() {
		return mods.applyToVal(baseValue);
	}
	float recalculateTotal() {
		//FIX: value should be recalculated at the very end of each tick, this is so that all the modifiers have been set back in place before calculating
		//then when value is used on next tick, its consistent wherever it is used.
		return value = this->afterMods();
	}
	float getBonus() {
		return value - baseValue;
	}

	void resetMods() {
		mods.reset();
	}

	float operator=(int& v) {
		return baseValue = static_cast<float>(v);
	}
	float operator=(float& v) {
		return baseValue = v;
	}
};