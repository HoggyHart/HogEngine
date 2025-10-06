#pragma once

#include <string>
//for resources holder
#include <map>
#include <vector>
//FIX: Resource inherit from Stat? resource could have modifier,
enum ResourceType { rsrcHitPoints, rsrcStamina, rsrcAbilityCasting };
typedef unsigned int Uint32;
struct Resource {
	std::string resourceName;
	float max;
	float cur;
	float regenRate;

	Resource(std::string n, float max = 1, float regenRate = 0) : resourceName(n), max(max), cur(max), regenRate(regenRate) {};

	void regenerate(Uint32 msToRegenerate) {
		cur += regenRate * msToRegenerate / 1000.0f;
		if (cur > max) cur = max;
	}

	void refill() {
		cur = max;
	}

	void empty() {
		cur = 0;
	}

	bool operator<(const float& val) {
		return cur < val;
	}
	float operator>(const float& val) {
		return this->cur > val;
	}
	bool operator==(const float& val) {
		return cur == val;
	}
	bool operator<=(const float& val) {
		return cur < val || cur == val;
	}
	float operator=(const float& val) {
		return this->cur = val;
	}
	float operator-=(const float& val) {
		return this->cur -= val;
	}
};

typedef std::map<ResourceType, std::vector<Resource>> Resources;
//struct Resources : std::map<ResourceType, std::vector<Resource>>{
//	Resources(ResourceType t, std::vector<Resource> v) : std::map<ResourceType, std::vector<Resource>>
//};