#pragma once
#include <functional>
#include <string>
#include <map>
#include <any>
class Entity;

class Behaviour {
private:
	std::function<void(Entity*, std::map<std::string, std::any>&) > behaviour;
public:

	std::map<std::string, std::any> behaviourVariables;

	Behaviour(std::function<void(Entity*, std::map<std::string, std::any>&)> behaviour, std::map<std::string, std::any> variables) : behaviour(behaviour) {
		behaviourVariables = variables;
	};
	void behave(Entity* performer) { behaviour(performer, behaviourVariables); }
};