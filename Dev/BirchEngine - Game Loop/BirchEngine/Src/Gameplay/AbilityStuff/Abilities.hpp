#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Gameplay/AbilityStuff/StandardAbilities.hpp"
#include <functional>
#include <map>
#include <any>

//include declaration for standard abilities
//#include "../Gameplay/ActionStuff/Abilities.hpp"

//Object for handling stats of effect, i.e. if ability spawns projectiles, AbilityStats will store {"ProjCount", unsinged int{ 5 }} for example
//these are to be used within PassiveEffect and ActiveEffect, not directly within the ABility object itself
typedef std::string StatName; //this should be replaced by enum for centralisation of stat names
typedef std::map<StatName, std::any> EffectStats;

class Ability;
//Params:
//Ability& : needed to access whether method is already active (Ability -> currentlyActive) and whether the activation condition is met ( Ability -> activationCriteria -> criteriaMet)
//			AbilitySet& can be accessed through Ability&. it is needed for access to abilityOwner, abilityUser, and rest of abilities (i.e. for disabling while this one is in use)
typedef std::function<void(Ability&)> EffectMethod;

typedef WHE_EventType PassiveEffectActivator;
typedef WHE_EventType PassiveEffectDeActivator;
typedef std::vector<WHE_EventType> PassiveEffectActivators;

//base struct NOT to be used for anything but inheritance
//implementations of ActivationMethod follow AbilityEffect as they need knowledge about AbilityEffect to initialise
//FIX: make ActivationMethod as part of Ability a union. CONTINUE: started declaration later in this file already
//CONTINUE: start of union implementation of ActivationMethod
enum ActivationType { atActive, atPassive };
struct ActivationMethod {
	ActivationType type;
	Ability* abilityToActivate{ nullptr };
	bool detectionInited{ false };
	bool detectionActive{ false };
	ActivationMethod() {};
	virtual ~ActivationMethod() {};

	virtual void initActivationDetection() {}
	virtual void deInitActivationDetection() {}
	virtual void blockDetection() {};
	virtual void unblockDetection() {};
};
struct ActiveActivationMethod;
struct PassiveActivationMethod;

//Ability MUST be used as part of an AbilitySet
struct AbilitySet;
class Ability {
public:
	AbilitySet* restOfSet{ nullptr };

	std::string abilityName{ "Unknown Ability" };
	std::string abilityDescription{ "Missing description." };

	ActivationMethod* activationCriteria{ nullptr }; //ptr to use virtual methods of child classes //FIX: probably good to replace with union at some point
	bool currentlyActive{ false }; //bool to be used by EffectMethod

	EffectMethod method{};
	EffectStats stats{};

	Ability() {};

	Ability(
		std::string name,
		std::string description,
		ActivationMethod* activation,
		EffectMethod effect,
		EffectStats stats
	) :
		abilityName(name),
		abilityDescription(description),
		method(effect),
		stats(stats)
	{
		setActivationMethod(activation);
	}
	Ability(Ability& a) :
		restOfSet{ nullptr },
		abilityName(a.abilityName),
		abilityDescription(a.abilityDescription),
		currentlyActive{ false },
		method(a.method),
		stats(a.stats)
	{
		setActivationMethod(a.activationCriteria);
		activationCriteria->detectionInited = false;
	}
	Ability(const Ability& a) :
		restOfSet{ nullptr },
		abilityName(a.abilityName),
		abilityDescription(a.abilityDescription),
		currentlyActive{ false },
		method(a.method),
		stats(a.stats)
	{
		setActivationMethod(a.activationCriteria);
		activationCriteria->detectionInited = false;
	}

	Ability& operator=(const Ability& a) {
		restOfSet = nullptr;
		abilityName = (a.abilityName);
		abilityDescription = (a.abilityDescription);
		currentlyActive = { false };
		method = (a.method);
		stats = (a.stats);
		setActivationMethod(a.activationCriteria);
		activationCriteria->detectionInited = false;
		return *this;
	}
	~Ability() {
		if (activationCriteria) {
			activationCriteria->deInitActivationDetection();
			delete activationCriteria;
		}
	}

	void setActivationMethod(ActivationMethod* method);
};

struct AbilitySet {
	//Owner : Entity that has the abilitySet as a component
	Entity* abilitySetOwner{ nullptr };
	//User : Entity making use of ability
	//		i.e. inherent ability of a creature : abilityUser = this->entity in AbilitySet::equipSetToEntity()
	//			 ability granted by wielder of an item : abilityOwner = given by parameter in AbilitySetComponent::equipSetToEntity();
	Entity* abilitySetUser{ nullptr };
	//some methods may affect the entityManager specifically instead of the user, i.e. an item may have a method to heal itself passively.
	//in these cases the user is the entityManager. in these cases it must be handled inside the effectMethod

	std::map<std::string, Ability> abilities{};

	AbilitySet() {};
	AbilitySet(AbilitySet& set) {
		for (std::pair<const std::string, Ability>& ability : set.abilities) {
			this->addAbility(ability.second);
		}
	}
	~AbilitySet() {
	}

	//changing owners means deleting the activation detection for the previous entityManager
	void changeOwner(Entity* e) {
		if (abilitySetOwner) deInitialiseActivationDetection();
		abilitySetOwner = e;
		if (abilitySetOwner) initialiseActivationDetection();
	}
	//SHOULD BE CALLED ONLY WHEN OWNER IS CHANGED, THIS ADDS INPUTCONTROLLER(active) / GAMEEVENTHANDLER(passive) STUFF
	void initialiseActivationDetection() {
		for (std::pair<const std::string, Ability>& ab : abilities) {
			if (ab.second.activationCriteria && !ab.second.activationCriteria->detectionInited) ab.second.activationCriteria->initActivationDetection();
			if (abilitySetUser == nullptr) ab.second.activationCriteria->blockDetection();
		}
	}
	//SHOULD ONLY BE CALLED WHEN CHANGING OWNER, SHOULD REMOVE INPUTCONTROLLER/GAMEEVENTHANDLER STUFF
	void deInitialiseActivationDetection() {
		for (std::pair<const std::string, Ability>& ab : abilities) {
			if (ab.second.activationCriteria && ab.second.activationCriteria->detectionInited) ab.second.activationCriteria->deInitActivationDetection();
		}
	}

	void addAbility(Ability& effect) {
		//FIX: name problems. should instead be added as "Action (2)" in case of two actions having same name.
		// could also be replaced with map? not sure what difference is between vector<pair> and map
		//add the ability
		abilities[effect.abilityName] = effect;
		abilities[effect.abilityName].restOfSet = this;
		if (abilitySetUser != nullptr) effect.activationCriteria->initActivationDetection();
	}
	void addAbility(const Ability& effect) {
		abilities[effect.abilityName] = effect;
		abilities[effect.abilityName].restOfSet = this;
		if (abilitySetUser != nullptr) effect.activationCriteria->initActivationDetection();
	}
	void removeAbility(std::string abilityID) {
		for (auto it = abilities.begin(); it != abilities.end(); ) {
			if (it->first == abilityID) {
				it = abilities.erase(it);

				if (it->second.activationCriteria->detectionInited) it->second.activationCriteria->deInitActivationDetection();
			}
			else {
				++it;
			}
		}
	}

	void equipToEntity(Entity* e) {
		//POTFIX: may be necessary to deactivate effects of abilities:
		//abilities.endEffects()
		abilitySetUser = e;
		//if now has no user, block activation detection
		if (abilitySetUser) for (std::pair<const std::string, Ability>& p : abilities) p.second.activationCriteria->unblockDetection();
		else for (std::pair<const std::string, Ability>& p : abilities) p.second.activationCriteria->blockDetection();
	}
};

struct ActiveActivationMethod : public ActivationMethod {
private:
	//ID of InputResponseMethod created by initialisation
	unsigned long long irm{ -1u };
public:

	SDL_Keycode activationButton{ SDLK_UNKNOWN };

	ActiveActivationMethod(SDL_Keycode button) : ActivationMethod(), activationButton(button) {
		type = atActive;
	}
	ActiveActivationMethod(ActiveActivationMethod& aam) :
		activationButton{ aam.activationButton }
	{
		type = atActive;
	}

	void initActivationDetection() override {
		//ability entityManager is usually the ItemStackEntity;
		Entity*& abilityOwner = abilityToActivate->restOfSet->abilitySetOwner;
		//if entityManager has no inputcontroller, add one
		if (!abilityOwner->hasComponent<InputController>()) abilityOwner->addComponent<InputController>();

		InputResponseMethod methodActivation{ [this](Entity* e, bool& methodActive, bool& comboActive) {this->abilityToActivate->method(*this->abilityToActivate); } };

		irm = abilityOwner->getComponent<InputController>().addControl(InputResponse{ methodActivation,false }, KeyComboInput{ {activationButton},true }).getID();

		detectionInited = true;
		detectionActive = true;
	}
	void deInitActivationDetection() override {
		//if not needed to be deInited via a lack of InputController to deInit from
		if (!abilityToActivate->restOfSet || !abilityToActivate->restOfSet->abilitySetOwner || !abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
			detectionInited = false; return;
		}
		abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().delControl(irm);
		detectionInited = false;
		detectionActive = false;
	}

	void blockDetection() override {
		if (!detectionActive) return; //if already blocked, skip
		if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
			detectionActive = false;
			return;
		}
		abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().blockControl(irm);
		detectionActive = false;
	}
	void unblockDetection() override {
		if (detectionActive) return; //if already unblocked, skip
		if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
			detectionActive = false;
			return;
		}
		abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().unblockControl(irm);
		detectionActive = true;
	}
};

struct PassiveActivationMethod : public ActivationMethod {
	PassiveEffectActivators activationEvents{};
	WHE_Event lastProcInfo{};

	unsigned int erm{ -1u };

	PassiveActivationMethod(PassiveEffectActivators acts) : ActivationMethod(), activationEvents(acts) {
		type = atPassive;
	}
	PassiveActivationMethod(PassiveActivationMethod& pam) :
		activationEvents{ pam.activationEvents }
	{
		type = atPassive;
	}
	void initActivationDetection() override {
		//ability entityManager is usually the ItemStackEntity;
		Entity*& abilityOwner = abilityToActivate->restOfSet->abilitySetOwner;
		//if entityManager has no inputcontroller, add one
		if (!abilityOwner->hasComponent<GameEventListenerComponent>()) abilityOwner->addComponent<GameEventListenerComponent>();

		EventResponseMethod methodActivation{ [this](Entity* e, WHE_Event ev, bool& methodActive) {this->lastProcInfo = ev; this->abilityToActivate->method(*this->abilityToActivate); } };
		//FIX: TEMPORARY IMPLEMENTATION OF RESPONSE TYPE.
		ResponseControl control{ EventResponse{methodActivation,responseListener, vectorHasVal(activationEvents,WHE_NoEvent)},activationEvents };
		erm = abilityOwner->getComponent<GameEventListenerComponent>().addControl(control).getID();

		detectionInited = true;
		detectionActive = true;
	}
	void deInitActivationDetection() override {
		//if not needed to be deInited via a lack of InputController to deInit from
		if (!abilityToActivate->restOfSet || !abilityToActivate->restOfSet->abilitySetOwner || !abilityToActivate->restOfSet->abilitySetOwner->hasComponent<GameEventListenerComponent>()) {
			detectionInited = false; return;
		}
		abilityToActivate->restOfSet->abilitySetOwner->getComponent<GameEventListenerComponent>().delControl(erm);
		detectionInited = false;
		detectionActive = false;
	}
};

union Activation {
	ActivationType type;

	ActiveActivationMethod active;
	PassiveActivationMethod passive;
};