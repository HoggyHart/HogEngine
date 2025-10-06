#include "InputController.hpp"
#include "Src/Gameplay/Entities.hpp"
#include "components.hpp"
#include "Src/Gameplay/ItemStuff/Inventory.hpp"
#include "Src/Utils/utils_SDL.hpp"
#include "Src/Utils/Camera.hpp"
//MOVEMENT METHODS
auto& MoveUp = [](Entity* e, bool& methodActive, bool& comboActive) {
	if (!methodActive && comboActive) { e->getComponent<MovementComponent>().velocity.y -= 1; methodActive = true; }
	if (methodActive && !comboActive) { e->getComponent<MovementComponent>().velocity.y += 1; methodActive = false; }
	};
auto& MoveLeft = [](Entity* e, bool& methodActive, bool& comboActive) {
	if (!methodActive && comboActive) { e->getComponent<MovementComponent>().velocity.x -= 1; methodActive = true; }
	if (methodActive && !comboActive) { e->getComponent<MovementComponent>().velocity.x += 1; methodActive = false; }
	};
auto& MoveDown = [](Entity* e, bool& methodActive, bool& comboActive) {
	if (!methodActive && comboActive) { e->getComponent<MovementComponent>().velocity.y += 1; methodActive = true; }
	if (methodActive && !comboActive) { e->getComponent<MovementComponent>().velocity.y -= 1; methodActive = false; }
	};
auto& MoveRight = [](Entity* e, bool& methodActive, bool& comboActive) {
	if (!methodActive && comboActive) { e->getComponent<MovementComponent>().velocity.x += 1; methodActive = true; }
	if (methodActive && !comboActive) { e->getComponent<MovementComponent>().velocity.x -= 1; methodActive = false; }
	};
auto& Sprint = [](Entity* e, bool& methodActive, bool& comboActive) {
	e->getComponent<MovementComponent>().speed.mods[statModBaseMultiplicative] += 1.0f;
	};
auto& Dash = [](Entity* e, bool& methodActive, bool& comboActive) {
	e->getComponent<MovementComponent>().speed.mods[statModBaseMultiplicative] += 1.0f;
	};
//IMPLEMENTATION OF MOVEMENT METHODS
const ControlScheme ControlSets::movementControls{
	Control{ InputResponse{MoveUp,false}, KeyComboInput{{SDLK_w},true} },
	Control{ InputResponse{MoveDown,false}, KeyComboInput{{SDLK_s},true} },
	Control{ InputResponse{MoveLeft,false}, KeyComboInput{{SDLK_a},true} },
	Control{ InputResponse{MoveRight,false}, KeyComboInput{{SDLK_d},true} },
	Control{ InputResponse{Sprint,false}, KeyComboInput{{SDLK_LCTRL},true} }
};


void InputController::loadCharacterControls() {
	/* // EXAMPLE OF TAP HOLD AND TOGGLE

	auto& WTap = [](Entity* c, bool& methodActive, bool& keyComboActive) {
		std::cout << "W tap call method active\n";
		keyComboActive = false;
	};
	auto& WHold = [](Entity* c, bool& methodActive, bool& keyComboActive) {
		std::cout << "W HOLD method active\n";
	};
	auto& WToggle = [](Entity* c, bool& methodActive, bool& keyComboActive) {
		if (keyComboActive) { methodActive = !methodActive; keyComboActive = false; }
		if(methodActive) std::cout << "W TOGGLE method active\n";
		};
	auto& ctrlW = [](Entity* c, bool& methodActive, bool& keyComboActive) {
		std::cout << "CTRL+E active\n";
		};
	this->controls.push_back(Control{ InputResponse{WTap,false}, KeyComboInput{{SDLK_w}} });
	this->controls.push_back(Control{ InputResponse{WHold,false}, KeyComboInput{{SDLK_w}} });
	this->controls.push_back(Control{ InputResponse{WToggle,false}, KeyComboInput{{SDLK_w}} });
	this->controls.push_back(Control{ InputResponse{ctrlW,false}, new KeyComboInput<2>{{SDLK_LCTRL, SDLK_w}} });*/

#pragma region CharacterControls

	for (const Control& c : ControlSets::movementControls) {
		controls.push_back(c);
	}

	//bleh
	auto& interact = [](Entity* e, bool& methodActive, bool& comboActive) {
		for (Entity* itrab : e->entityManager.getGroup(EntityGroup::groupInteractables)) {
			if (itrab->getComponent<SpatialComponent>().getCenter().distance(e->getComponent<SpatialComponent>().getCenter()) < 150) {
				itrab->getComponent<InteractableComponent>().interact(e);
				comboActive = false;
				break;
			}
		}
		};
	this->controls.push_back(Control{ InputResponse{interact,false},KeyComboInput{{SDLK_e},true } });
#pragma endregion
}

void InputController::loadCameraControls()
{
	
}

void InputController::loadSystemControls() {
	auto& pauseMenu = [this](Entity* e, bool& methodActive, bool& comboActive) {
		static Entity* pauseEntity = nullptr;
		if (!methodActive && comboActive) {
			pauseEntity = &this->getOwner()->entityManager.addEntity<Entity>();
			pauseEntity->makeDisplayEntity();
			SDL_Color white{ 255,255,255,255 };
			pauseEntity->addComponent<TextComponent>(CENTER, "PAUSED", "Consolasx24", white);
			pauseEntity->addGroup(EntityGroup::groupGUIMenu);
			Game::paused = true;
			methodActive = true;
			comboActive = false; //combo will need repressing to reactivate
		}
		else if (methodActive && comboActive) {
			pauseEntity->destroy();
			Game::paused = false;
			pauseEntity = nullptr;
			methodActive = false;
			comboActive = false;
		}
		};
	auto& hitboxes = [](Entity* e, bool& methodActive, bool& comboActive) {
		Game::displayHitboxes = !Game::displayHitboxes;
		comboActive = false;
		};
	auto& exitApplication = [](Entity* e, bool& methodActive, bool& comboActive) {
		Game::isRunning = false;
		};
	
	auto& debugEntityInfo = [](Entity* e, bool& am, bool& ca) {
		//find entity hovered over by mouse
		// find mouse pos in-world
		// hitbox collision with mouse pos (i.e. SDL_Rect{x,y,0,0}) 
		//pos must be affected by camera zoom, since cam may be {0,0,10,10} (very zoomed in), but mouse x will be display-relevant so up to 1000 (atm)
		int x;
		int y;
		SDL_GetMouseState(&x, &y);
		SDL_Rect mousePos{ x/Game::camera.zoomAmount + Game::camera.x,y / Game::camera.zoomAmount + Game::camera.y,0,0 };
		for (Entity*& et : e->entityManager.getEntities()) {
			if (et->hasComponent<SpatialComponent>()) {
				SpatialComponent& s = et->getComponent<SpatialComponent>();
				if (et->isGameEntity()) {
					if (s.position.x + s.width * s.scale > mousePos.x
						&& s.position.x < mousePos.x
						&& s.position.y + s.height * s.scale > mousePos.y
						&& s.position.y < mousePos.y) {
						if (et->hasComponent<TileComponent>()) {
							TileComponent& tc = et->getComponent<TileComponent>();
								// display info
							std::cout << "Hovering over " << s.position << "; " << s.width * s.scale << ", " << s.height * s.scale << "\n";
							std::cout << "Drawn at      " << tc.dstRect<<"\n\n";
						}
					}
				}
				else {
					//if display entity, should use raw mousePos, not camera affected ver
				}
			}

		}
		};
	this->controls.push_back(Control{ InputResponse{debugEntityInfo,false},KeyComboInput{{SDLK_i},true} });
	auto& toggleDebugMode = [](Entity* e, bool& am, bool& ca) {
		
		};
	this->controls.push_back(Control{ InputResponse{hitboxes,false},KeyComboInput{{SDLK_h},true } });
	this->controls.push_back(Control{ InputResponse{pauseMenu,false},KeyComboInput{{SDLK_ESCAPE},true } });
}