#pragma once

#include "Src/Game Management/Game.hpp"
#include "Src/ECS/ECS.hpp"
#include <map>
#include <functional>
#include "Src/Utils/utils.hpp"

//eventHandle()
//	when a key is pressed try to activate combos with that key in it

//update()
//	if(method.isActive()) method();
//	else if(method.isNOTactive() && keyCombo.isActive()) method();
//methodActive should control:
//				- Takes over if the method is being run or not. i.e. combo is activated -> method decides whether to methodActive{true} or leave it dependant on the keyCombo being active

//combination of methodActive and keyComboActive can be used to make TAP methods, HOLD methods, or TOGGLE methods
// when a method is called for the first time, methodActive = false adn keyComboActive=true;
//TAP (i.e.  1 time std::cout):
//		if( !methodActive && keyComboActive) { doStuff ... keyComboActive=false; }
//HOLD (i.e. std::cout while button held):
//		no handling, if combo is activated method is called, dont fiddle with either and when combo is not active method will not be called
//TAP, HOLD AND RELEASE (i.e. holding but do something special at start / end):
//		if( !methodActive && keyComboActive) { startMethod ... methodActive=true;}
//		else if( methodActive && keyComboActive) { continueMethod }
//		else if( methodActive && !keyComboActive) { endMethod ... methodActive=false; }
//TOGGLE (i.e. std::cout after press 1, stop after press 2):
//		if( keyComboActive) { startMethod ... methodActive = !methodActive; keyComboActive=false; }
//
typedef std::function<void(Entity*, bool& methodActive, bool& keyComboActive)> InputResponseMethod;

struct KeyComboInput {
	bool comboExclusiveKeys{ true }; //i.e. if Q + E are pressed, should they do the QE combo method ONLY (true) or should they do the QE combo AND the Q + E solo methods as well (false).
	bool ordered{ true }; //keys must be pressed in that order i.e. {LCTRL, Q} cannot be activated by Q then LCTRL press if true
	bool active{ false }; //if keyCombo is currently active, doesnt necessarily mean keys are still held, i.e. if you press a button and it sticks down, the button is active, but not being pressed
	bool ignoresModifiers{ false }; //can be activated despite LCTRL being pressed for example.
	//									i.e. if Q shoots, and ctrl+Q jumps, and you dont want to shoot when you do ctrl+Q, set this to false
	bool isActive() const { return active; }

	std::vector<SDL_Keycode> keys;
	KeyComboInput(std::vector<SDL_Keycode> combo, bool ignrMods) : keys(combo) { ignoresModifiers = ignrMods; }

	void match(std::vector<SDL_Keycode>& heldButtons) {
		std::size_t goal{ keys.size() };
		std::size_t matched{ 0 };
		std::array<SDL_Keycode, 6> modifiers{ SDLK_LCTRL, SDLK_RCTRL, SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LALT, SDLK_RALT };
		if (ordered) {
			//requires that heldButtons be in order of which was pressed first.
			std::vector<SDL_Keycode>::iterator kit = heldButtons.begin();
			while (kit != heldButtons.end() && matched != goal) {
				//if the key matches, +1 to matched keys in combo
				if (*kit == keys[matched]) ++matched;
				//else if it didnt match and it was a modifier key,
				else if (!ignoresModifiers && arrayHasVal(modifiers, *kit) && !vectorHasVal(keys, *kit)) {
					break;
				}
				++kit;
			}
		}
		else {
			for (SDL_Keycode& k : keys) {
				//for each key in combo, if it is found in the keys pressed, add 1 to the amount matched
				if (std::find_if(heldButtons.begin(), heldButtons.end(), [k](SDL_Keycode& b) {return k == b; }) != heldButtons.end()) ++matched;
			}
		}

		active = matched == goal;
	};

	bool hasKey(SDL_Keycode k) {
		return std::find(keys.begin(), keys.end(), k) != keys.end();
	}
};

// stores response method and whether it is to be-executed every update tick
//		- method itself should handle whether it is dependant on the key combo being maintained to keep running i.e. key is pressed once, method keeps repeating OR key is pressed once, method stops when key is released
struct InputResponse {
private:
	InputResponseMethod method;

	bool active{ false };
	bool activeByDefault{ false };
public:
	bool blocked{ false };
	InputResponse(InputResponseMethod m, bool activeByDefault) : method(m), active(activeByDefault), activeByDefault(activeByDefault) {}
	InputResponse(const InputResponse& ir) : method(ir.method), active(ir.activeByDefault), activeByDefault(ir.activeByDefault), blocked(ir.blocked) {}
	void doMethod(Entity* inputter, bool& comboActive) {
		if (!blocked) method(inputter, active, comboActive);
	}
	bool isActive() const {
		return active;
	}
};

struct Control : public std::pair<InputResponse, KeyComboInput>, public Unique_ID_Object {
	Control(InputResponse ir, KeyComboInput ib) : std::pair<InputResponse, KeyComboInput>(ir, ib), Unique_ID_Object() {}
};
typedef std::vector<Control> ControlScheme;
typedef std::vector<SDL_Keycode> Inputs;

namespace ControlSets {
	extern const ControlScheme movementControls;
	extern const ControlScheme hotbarControls;
}

class InputController : public EComponent {
protected:
	bool globalInputBlocked{ false };

	Inputs heldButtons{};
	ControlScheme controls{};

public:
	InputController(Entity& e) : EComponent(e) {
		this->getOwner()->addGroup(EntityGroup::groupInputHandlers);
	};
	InputController(Entity& e, InputController& ic) : EComponent(e),
		controls(ic.controls),
		globalInputBlocked(ic.globalInputBlocked) {
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<InputController>(*this);
	}
	~InputController() {
		this->getOwner()->delGroup(EntityGroup::groupInputHandlers);
	}

	Control& addControl(InputResponse ir, KeyComboInput ki) {
		controls.push_back(Control{ ir, ki });
		return controls.back();
	}
	void delControl(unsigned int ctrlID) {
		//find control with given ID
		ControlScheme::iterator it = std::find_if(controls.begin(), controls.end(), [ctrlID](Control& r) {return r.getID() == ctrlID; });
		//if control found
		if (it != controls.end()) {
			controls.erase(it);
		}
	}
	void blockControl(unsigned int ctrlID) {
		//may have to deactivate it first. would need to make sure controls contains ctrl first.
		ControlScheme::iterator it = std::find_if(controls.begin(), controls.end(), [ctrlID](Control& r) {return r.getID() == ctrlID; });
		it->first.blocked = true;
	}
	void unblockControl(unsigned int ctrlID) {
		ControlScheme::iterator it = std::find_if(controls.begin(), controls.end(), [ctrlID](Control& r) {return r.getID() == ctrlID; });
		it->first.blocked = false;
	}
	void loadCharacterControls();
	void loadCameraControls();
	void loadSystemControls();

	void fixedUpdate(bool EOI) override { //event queue will be cleared by Game::handleSDLEvents so no need to worry about the passing of Game::event.type unless an input deals with SDL_POLLSENTINEL event (will not happen)
		ControlScheme::iterator it = controls.begin();
		while (it != controls.end()) {
			//if method is active, call method with params of entity in control and the combo's activity state
			if (it->first.isActive()) it->first.doMethod(this->getOwner(), it->second.active);
			else if (it->second.isActive()) it->first.doMethod(this->getOwner(), it->second.active);
			++it;
		}
	}

	//for immediate response to SDL_Events
	void eventOccuredResponse(SDL_Event& ev) {
		//respond to the input event only by executing the method for which that key is associated with,

		//"improved"
		if (!globalInputBlocked) {
			// repeat inputs for the same button press must be handled by only doing the buttonPress method if the button is NOT held down
			// i.e. if holding K down, there will be repeated SDL_KEYDOWN events for K while it is held. To prevent the response being triggered faster than desired (i.e. more than once per update tick), only trigger the immediate response here if the button is NOT already pressed down.
			switch (ev.type) {
				//if key event
			case SDL_KEYDOWN:
				//if button is not already held
				//FIX: use ev.key.repeat instead? i.e. if (ev.key.repeat) return;
				if (std::find(heldButtons.begin(), heldButtons.end(), ev.key.keysym.sym) == heldButtons.end()) {
					//register it as pressed
					heldButtons.push_back(ev.key.keysym.sym);
					//if there are any controls that use this key, see if they would be activated by this keypress
					for (auto& ir : controls) if (ir.second.hasKey(ev.key.keysym.sym)) ir.second.match(heldButtons);
				}
				break;
			case SDL_KEYUP:
				for (auto& ir : controls) if (ir.second.hasKey(ev.key.keysym.sym)) ir.second.active = false;
				heldButtons.erase(std::remove(heldButtons.begin(), heldButtons.end(), ev.key.keysym.sym), heldButtons.end());
				break;

				//if mouse button clicked
			case SDL_MOUSEBUTTONDOWN:
				if (std::find(heldButtons.begin(), heldButtons.end(), ev.button.button) == heldButtons.end()) {
					heldButtons.push_back(Sint32{ ev.button.button });
					for (auto& ir : controls) if (ir.second.hasKey(Sint32{ ev.button.button })) ir.second.match(heldButtons);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				for (auto& ir : controls) if (ir.second.hasKey(ev.button.button)) ir.second.active = false;
				heldButtons.erase(std::remove(heldButtons.begin(), heldButtons.end(), Sint32{ ev.button.button }), heldButtons.end());
				break;

			case SDL_MOUSEMOTION:
				break;

			case SDL_MOUSEWHEEL:
				
				break;

			default:
				break;
			}
		}
	}

	void blockInput(bool block) {
		if (block) {
			for (auto& p : controls) {
				p.second.active = false;
			}
			heldButtons.clear();
		}
		globalInputBlocked = block;
	}

	void clearInputs() {
		heldButtons.clear();
	}
};