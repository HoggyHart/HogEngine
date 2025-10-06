#pragma once
#include "Src/ECS/ECS.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Gameplay/Game Systems/GameEvents.hpp"
#include "Src/Utils/utils.hpp"
#include <functional>
typedef std::function<void(Entity*, WHE_Event& ev, bool& methodActive)> EventResponseMethod;

/// <summary of event handling>
/// Abilities have 2 event stages:
///		AbilityStart: request to use ability
///		AbilityConfirmed: uses the ability
/// At end of modifying/reactive GameEventListeners an EventController eventlistener will listen to the currently polled event, and if Event.deprecated == false (i.e. another event hasn't updated it , it
/// E: Event. EQ: EventQueue. OL = Other Listeners. GL = Global Subscriber. (O) = front event in deprecation chain. (D-) = deprecated. (D+) = dupe from deprecated event
///
/// .dAD() = .deprecateAndDelay() .
/// // this will deprecate this event and make an un-deprecated copy at the end of the queue
/// // this is to be called when another event is trying to deprecate it BUT it is subject to being cancelled out
/// // the other event must store a pointer to the un-deprecated version of the .dAD()'d event so that if it gets .dAD()'d the event that it .dAD()'d also gets re-dAD()'d
///
///
/// //	Consider this (using a Functional Event Queue i.e. excluding deprecated events)
/// //		event A tries to happen. FEQ = {Ar}
/// //		listener B requests to stop event Ar from happening so it dAD()'s it. FEQ = {Br,Ar}
/// //		listener C requests to stop event Br from happening so it dAD()'s it (i.e. tries to ALLOW event Ar to succeed despite event Br's interference)
/// //			Without linked dAD() tracking: FEQ = {Ar,Cr,Br}.
///					- this allows for Ar to succeed even if event Cr is going to get cancelled out by something else that would prevent Ar from happening
///	//			With linked dAD() tracking: FEQ = {Cr,Br,Ar}
/// //				- this means that Cr has a chance to get deprecated and allow for Ar to get blocked again
///			listener D straight up deprecates event C: FEQ{Br,Ar}
///				Now C cannot cancel B, so B goes off, which deprecates A with no dAD(). This stops A from happening.
///		Alternatively (FIX):
///			listener D deprecates event Br: FEQ = {Cr, Ar}
///				now Cr is trying to cancel an event which is deprecated, but the request isnt deprecated, should Cs event happen (successful request event)? i think not. ill think about it
///
/// ************INTERACTION START**************
/// //e1 hits e2 creating damage event
///
/// E = ...; EQ = {e2DamageEvent(O),...}
/// OL and GL hear -> other events happen.
///
/// E = e2DamageEvent(O); EQ {...}
/// // e2 requests to negate the damage: abilityrequest is queued. damage event is deprecated and a duplicate is created at the back of the queue
/// e2 hears -> e2AbilityStart AND E.dAD(); EQ = {..., e2AbilityStart(O), e2DamageEvent(D+)}
/// OL+GL hear E.deprecated -> ignore
///
/// E = ...; EQ = {e2AbilityStart(O), e2DamageEvent(D+)}
/// OL and GL hear -> other events happen.
///
/// E = e2AbilityStart(O); EQ{e2DamageEvent(D+), ...};
/// e1 hears -> E.dAD() AND e1AbilityStart; EQ = {e2DamageEvent(D-), ..., e1AbilityStart(O), e2AbilityStart(D+), e2DamageEvent(D+)} // e1 requests to cancel that mitigate damage ability: abilityRequest queued. dupe and push back e2Ability
/// GL hears -> E.deprecated == true: do nothing
///
/// E = e2DamageEvent(D-); EQ = {..., e1AbilityStart(O), e2AbilityStart(D+), e2DamageEvent(D+)};
/// E.deprecated -> skip;
///
/// E = ...; EQ = { e1AbilityStart(O), e2AbilityStart(D+), e2DamageEvent(D+)};
/// OL and GL hear -> other events happen.
///
/// E = e1AbilityStart(O); EQ = {, e2AbilityStart(D + ), e2DamageEvent(D + ) };
/// GL hears -> not deprecated -> E.ability(E.params) -> e2AbilityStart(D+).deprecate(); EQ = {e2AbilityStart(D-), e2DamageEvent(D+)};
///
/// E = e2AbilityStart(D-); EQ = { e2DamageEvent(D + ) };
/// GL hears -> deprecated -> ignore;
///
/// E = e2DamageEvent(D+); EQ ={}
/// GL hears -> not deprecated -> e2.takeDamage();
///
/// *********END INTERACTION*********
/// </summary>

//FIX: EventResponders are divided into type;
//	MODIFIERS should go first.
//		- modifiers include responders that change the values of an event and responders that attempt to deprecate an event
//	LISTENERS/ go second:
//		- these perform processes that do not directly affect the event they respond to
//		- they include trackers and reactions that cause an unrelated event to be queued
//
enum ResponseType { responseModifier, responseListener };

//CONTINUE: finish modifier/listener differentiation code. need to add way to categorise the two on creation
struct EventResponse {
private:
	ResponseType type;
	EventResponseMethod method{ [](Entity*, WHE_Event& ev, bool& methodActive) {return; } };
	WHE_Event lastProcInfo{};
	bool active{ false };
	bool activeByDefault{ false };
public:
	bool blocked{ false };
	//should be a bit more secure to prevent modifier being passed in as listener
	//should repeat collisionCallback design, where Subscriber uses an ID to indicate what method it's using.
	//the map storing the methods should store them as pairs of method and type for security,
	// So:
	//		ID passed as parameter -> grab pair from map with ID, get type from pair
	EventResponse(EventResponseMethod m, ResponseType t, bool activeByDefault) : method(m), type(t), active(activeByDefault), activeByDefault(activeByDefault) {}
	EventResponse(EventResponseMethod m, EventResponse& er) : method(er.method), type(er.type), active(er.activeByDefault), activeByDefault(er.activeByDefault) {}

	void doMethod(Entity* inputter, WHE_Event* ev) {
		if (blocked) return;

		//if modifier method, it should only respond to an active event, so pass the live event as parameter
		if (type == responseModifier) {
			method(inputter, *ev, active);
		}

		//if listener,
		//	if event trigger: store a copy of the event and pass that as the parameter
		//	if no event: just call the method with the stored last event's details
		else {
			if (ev) lastProcInfo = *ev;
			method(inputter, *ev, active);
		}
	}
	inline ResponseType getType() const { return type; }
	inline bool isActive() const {
		return active;
	}
};

struct ResponseControl : public std::pair<EventResponse, std::vector<WHE_EventType>>, public Unique_ID_Object {
	ResponseControl(EventResponse er, std::vector<WHE_EventType> acts) : std::pair<EventResponse, std::vector<WHE_EventType>>(er, acts), Unique_ID_Object() {}
};
typedef std::vector<ResponseControl> ResponseControls;
typedef std::map<ResponseType, ResponseControls> EventResponses;

class GameEventListenerComponent : public EComponent {
protected:

	bool globalInputBlocked{ false };

	EventResponses responses{};

public:
	GameEventListenerComponent(Entity& e) : EComponent(e) {
		this->getOwner()->addGroup(EntityGroup::groupEventHandlers);
	}
	GameEventListenerComponent(Entity& e, GameEventListenerComponent& lc) : EComponent(e),
		responses(lc.responses) {
	};
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<GameEventListenerComponent>(*this);
	}
	ResponseControl& addControl(ResponseControl ctrl) {
		responses[ctrl.first.getType()].push_back(ctrl);
		return responses[ctrl.first.getType()].back();
	}

	//FIX: DOES NOT WORK
	void delControl(unsigned int ctrlID) {
		//find control with given ID
			//may have to deactivate it first. i.e. passive on enter combat give shield -> block passive mid combat -> remove shield?
		ResponseControls::iterator it = std::find_if(responses[responseModifier].begin(), responses[responseModifier].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it != responses[responseModifier].end()) {
			responses[responseModifier].erase(it);
			return;
		}

		ResponseControls::iterator it2 = std::find_if(responses[responseListener].begin(), responses[responseListener].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it2 != responses[responseListener].end()) {
			responses[responseListener].erase(it2);
			return;
		}
	}
	void fixedUpdate(bool EOI) override {
		//NOTE: responseModifier has been left out here because a modifier is an event-based method i.e. it should only do stuff when responding to an active event.

		//Listeners don't modify an active event so they can execute their methods without needing a current event to trigger it
		//for (ResponseControl& r : responses[responseListener]) {
		//	if (r.first.isActive()) {
		//		break; //FIX: debug break, im just not sure this is how it should be. listeners hear and react, the
		//		r.first.doMethod(this->getOwner(), nullptr);
		//	}
		//}
	}
	void blockControl(unsigned int ctrlID) {
		//may have to deactivate it first. i.e. passive on enter combat give shield -> block passive mid combat -> remove shield?
		ResponseControls::iterator it = std::find_if(responses[responseModifier].begin(), responses[responseModifier].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it != responses[responseModifier].end()) {
			it->first.blocked = true;
			return;
		}

		ResponseControls::iterator it2 = std::find_if(responses[responseListener].begin(), responses[responseListener].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it2 != responses[responseListener].end()) {
			it2->first.blocked = true;
			return;
		}
	}
	void unblockControl(unsigned int ctrlID) {
		ResponseControls::iterator it = std::find_if(responses[responseModifier].begin(), responses[responseModifier].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it != responses[responseModifier].end()) {
			it->first.blocked = true;
			return;
		}

		ResponseControls::iterator it2 = std::find_if(responses[responseListener].begin(), responses[responseListener].end(), [ctrlID](ResponseControl& r) {return r.getID() == ctrlID; });
		if (it2 != responses[responseListener].end()) {
			it2->first.blocked = true;
			return;
		}
	}

	//FIX: merge these two, use a param to say which responses to use
	//for immediate response to SDL_Events
	void eventOccuredModifierResponse(WHE_Event& ev) {
		//respond to the input event only by executing the method for which that key is associated with,

		if (!globalInputBlocked) {
			// repeat inputs for the same button press must be handled by only doing the buttonPress method if the button is NOT held down
			// i.e. if holding K down, there will be repeated SDL_KEYDOWN events for K while it is held. To prevent the response being triggered faster than desired (i.e. more than once per update tick), only trigger the immediate response here if the button is NOT already pressed down.
			for (ResponseControl& r : responses[responseModifier]) {
				//if 2nd (activator) or 3rd (deactivator) val of tuple matches event type;
				if (vectorHasVal(r.second, ev.type)) {
					r.first.doMethod(this->getOwner(), &ev);
				}
			}
		}
	}
	//for immediate response to SDL_Events
	void eventOccuredListenerResponse(WHE_Event& ev) {
		//respond to the input event only by executing the method for which that key is associated with,

		if (!globalInputBlocked) {
			// repeat inputs for the same button press must be handled by only doing the buttonPress method if the button is NOT held down
			// i.e. if holding K down, there will be repeated SDL_KEYDOWN events for K while it is held. To prevent the response being triggered faster than desired (i.e. more than once per update tick), only trigger the immediate response here if the button is NOT already pressed down.
			for (auto& r : responses[responseListener]) {
				//if 2nd (activator) or 3rd (deactivator) val of tuple matches event type;
				if (vectorHasVal(r.second, ev.type)) {
					r.first.doMethod(this->getOwner(), &ev);
				}
			}
		}
	}
};