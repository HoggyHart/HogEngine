#pragma once
#include "components.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Gameplay/CreatureStuff/Behaviour.hpp"

//FIX: should be made up of modules i.e. navigation module, combat module, whatever else.
// tbh im not sure exactly how it would work because the combat module would surely affect the navigation
//	maybe: navigation would have a switch case for what state the combat module is in? if module is marked in combat then navigation switches to combat navigation? idk

//FIX: this was a quick move from MovemmentComponent. verify/ redo navigation control stuff
typedef std::function<void(Entity* thisEntity)> NavigationMethod;

struct NavigationControl {
	NavigationMethod navigationControl;;

	void setOnPath(NavigationMethod pf) { navigationControl = pf; }
	void navigate(Entity*& navigatingEntity) {
		navigationControl(navigatingEntity);
	}
};

class BehaviourComponent : public EComponent {
private:
	Behaviour* behaviour;
public:
	BehaviourComponent(Entity& e, Behaviour* b) : EComponent(e), behaviour(b) {};
	BehaviourComponent(Entity& e, BehaviourComponent& bc) : EComponent(e), behaviour(bc.behaviour) {}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<BehaviourComponent>(*this);
	}
	void fixedUpdate(bool EOI) override {
		behaviour->behave(this->getOwner());
	}
};

typedef std::function<void()> ActionControl;
class BehaviourComponent2 : public EComponent {
private:
	NavigationControl* mc{ nullptr };
	ActionControl* ac{ nullptr };
public:
	BehaviourComponent2(Entity& e, std::string moduleName) : EComponent(e) {}
	BehaviourComponent2(Entity& e, NavigationControl* navCon, ActionControl* actCon) : EComponent(e) {}
	BehaviourComponent2(Entity& e, BehaviourComponent2& bc) : EComponent(e), mc(bc.mc), ac(bc.ac) {}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<BehaviourComponent2>(*this);
	}
};