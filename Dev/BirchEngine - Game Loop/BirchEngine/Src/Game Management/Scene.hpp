#pragma once

#include "Game.hpp"
#include "Src/Gameplay/Entities.hpp"
#include "Src/Utils/NodePubSub.hpp"
#include "Src/Gameplay/AbilityStuff/Abilities.hpp"
#include "Src/Gameplay/ItemStuff/Item.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
#include "Src/Gameplay/WorldStuff/Map.hpp"
#include "Src/Utils/Camera.hpp"
#include "Src/Utils/Timer.hpp"
class Scene
{
	const char* sceneFolder;
public:
	Entity* systemController{ nullptr };
	Map map{};

	Scene(const char* sceneFolderPath) : sceneFolder(sceneFolderPath) {
	}

	void loadScene(Manager& entityManager) {
		//unpack scene folder here
			//import map data from file (size and stuff)
			//
			//import entities from file (including player entity) //map mgith get loaded here? would preferably be via importing map data but since tiles are entities...
			//	// maybe when saving a scene only save non-map/gui entities
		//set camera as part of loading process

		//this will recalculate the renderBuckets displayAreas. find a nicer way i think 
		entityManager.refresh();
#pragma region bootlegLoading
		if (sceneFolder == "scenes/mainmenu/") {
			Game::paused = true;
			SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
			Entity* e = &entityManager.addEntity<SimpleButtonEntity>(static_cast<int>(Game::camera.w / 2.0f), static_cast<int>(Game::camera.h / 2.0f), static_cast<int>(Game::camera.w * 0.2f), static_cast<int>(Game::camera.h * 0.1f), [](bool& b, int p) {
				if (b && !p) {
					Game::loadScene("DungeonEntrance");
				}
				else if (!b && p) { b = true; }
				});

			SDL_Color c{ 0,0,0,255 };
			e->addComponent<TextComponent>(CENTER, "Start Game", "Consolasx24", c);
			e->addGroup(EntityGroup::groupGUIMenu);
		}
		else if (sceneFolder == "scenes/startzone/") {
			Game::paused = false;

			systemController = &entityManager.addEntity<Entity>();
			InputController* systemControls = &systemController->addComponent<InputController>();
			systemControls->loadSystemControls();
			systemControls->loadCameraControls();

			map = Map("mapTiles", 2.0f, 32);
			map.LoadMap("assets/Maps/map.txt", 32, 32, &entityManager);
			entityManager.update();
			auto& p = entityManager.addEntity<PlayerEntity>();

			Item item{ baseballBat };
			item.addUse(*StandardAbilities::shootAtCursor);
			item.addUse(*StandardAbilities::swingBat);
			item.addUse(*StandardAbilities::onKillTest);
			entityManager.addEntity<ItemStackEntity>(item).addToWorld(400.0f, 400.0f, 32, 32, 1.0f);

		}
		else if (sceneFolder == "scenes/debugArea/") {
			Game::paused = false;

			systemController = &entityManager.addEntity<Entity>();
			InputController* systemControls = &systemController->addComponent<InputController>();
			systemControls->loadSystemControls();

			map = Map("mapTiles", 2.0f, 32);
			map.LoadMap("assets/Maps/debugMap.txt", 128, 128, &entityManager);

			auto& p = entityManager.addEntity<PlayerEntity>();

			Item item{ baseballBat };
			item.addUse(*StandardAbilities::shootAtCursor);
			item.addUse(*StandardAbilities::swingBat);
			item.addUse(*StandardAbilities::onKillTest);

			Item destroyer{ gun };
			destroyer.addUse(*StandardAbilities::destructionWave);

			entityManager.addEntity<ItemStackEntity>(destroyer).addToWorld(250.0f, 250.0f, 32, 32, 1.0f);
			entityManager.addEntity<ItemStackEntity>(item).addToWorld(400.0f, 250.0f, 32, 32, 1.0f);

			SDL_Color& color = *new SDL_Color{ 255,0,0,255 };
			Entity& te{ entityManager.addEntity<FloatingTextEntity>(0,0,"Speed","Consolasx24",color) };
			te.getComponent<TextComponent>().allignment = Allignment::TOP_LEFT;
			te.makeDisplayEntity();

			auto& c = te.addComponent<GameEventListenerComponent>();
			Vector2D* oldPos = new Vector2D{};
			Vector2D* newPos = new Vector2D{};
			c.addControl(ResponseControl{ EventResponse{
				[&p,oldPos,newPos](Entity* e, WHE_Event& ev, bool&) {
					if (ev.event.move.e == &p) {
						*newPos = p.getComponent<SpatialComponent>().position;
						float d = newPos->distance(*oldPos);
						*oldPos = *newPos;
						d = d / (GameTime::fixedDeltaTime/1000.0f);

						e->getComponent<TextComponent>().changeText(std::to_string(d)+"m/s");
					}
				},ResponseType::responseListener,true},
				std::vector<WHE_EventType>{WHE_EntityMove}
				});

			EnemyEntity& enemy{ entityManager.addEntity<EnemyEntity>(Vector2D{  64.0f,  64.0f }) };
			enemy.destroy();
			
			/******PUB / SUB EXAMPLE******/ //FIX: give publisher a setCallback method perhaps so it can address itself
		//	Publisher<EnemyEntity*>* pub{ nullptr };
		//	pub = new Publisher<EnemyEntity*>{ "firstEnemy",0,[&enemy](Publisher<EnemyEntity*>& thisP) {if (!enemy.isActive()) thisP.deactivate(); return new EnemyEntity* {&enemy}; } };

			//new Subscriber<EnemyEntity*>{ "firstEnemy",[](EnemyEntity*& e) {std::cout << "Entity active: " << e->isActive() << "\n"; } };
		}
#pragma endregion
		entityManager.refresh();
	}

};