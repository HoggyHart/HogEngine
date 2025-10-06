#include "Game.hpp"

//game running / general
#include <thread>
#include <chrono>
#include "Src/Utils/utils.hpp"
#include "Src/Utils/Timer.hpp"


//game uses
#include "Src/Game Management/GameTime.hpp"
#include "Src/Gameplay/Game Systems/GameEvents.hpp"
#include "Src/Gameplay/WorldStuff/Map.hpp"
#include "Src/ECS/components.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
#include "ObjectManagers/AssetManager.hpp"
#include "TextureManager.hpp"
#include "Src/Gameplay/Entities.hpp"
#include "Src/Gameplay/Game Systems/EnemySpawnManager.hpp"
#include "Scene.hpp"
#include "Src/Gameplay/Game Systems/EntityGroups.hpp"
#include "Src/Gameplay/Game Systems/CollisionHandler.hpp"

const std::pair<EntityGroup, EntityGroup> drawGroupsSaE{ EntityGroup::groupMap,EntityGroup::groupGUIMenu };

int Game::TPS;
SDL_Renderer* Game::renderer = nullptr;
SDL_Event Game::event;
WHE_Event Game::gevent;
StaticGameDataManager* Game::assetManager;

bool Game::isRunning = false;
ViewPort Game::camera;
Uint32 Game::gameTimePerTick = 0;
Uint32 Game::gameTime = 0;
bool Game::paused = false;

Scene* currentScene;
extern Manager entityManager{};
EnemySpawnManager enemySpawner{ entityManager,0,0 };

bool Game::displayHitboxes = false;

bool trackersEnabled = false;
Game::Game()
{
}

Game::~Game()
{
}

void Game::init(const char* title, int width, int height, bool fullscreen, const int TPS)
{
	this->TPS = TPS;
	GameTime::setFixedDeltaTime(1000/TPS); //1000 ms divided by num of ticks per second = game time per tick (ignoring speed mult) i.e. 1000/20 = 50ms between each tick
	int flags = 0;

	if (fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		window = SDL_CreateWindow(title, 0, 100, width, height, flags);
		Game::camera = { 0,0,width,height };
		Game::renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		}

		isRunning = true;
	}
	if (TTF_Init()) {
		printf("TTF_Init: %s\n", TTF_GetError());
		exit(2);
	}

	assetManager = new StaticGameDataManager();

	loadScene("Debug");

}

void Game::run(const unsigned int FPS) {
	const Uint32 frameDelay = 1000 / FPS; //milliseconds between each frame displayed.
	Uint32 frameStart = 0;

	const Uint32 tickDelay = 1000 / Game::TPS; //milliseconds between each frame displayed.
	Uint32 lastUpdate = 0;

	Timer::startTimer("totalRuntime");
	Uint32 lastRecord{ SDL_GetTicks() };

	Uint32 defragStart = 0;

	while (this->isRunning) {

		//GameTime::updateDeltaTime();
			
		//handle events
		Timer::startTimer("handleSDLEvents");
		this->handleSDLEvents();
		Timer::stopTimer("handleSDLEvents");
			
		//update
		
		if (SDL_GetTicks() - lastUpdate >= tickDelay) {			
			Timer::startTimer("updateLoop");
			lastUpdate = SDL_GetTicks();
			GameTime::updateDeltaTime();
			this->update();
			GameTime::resetAccumulated();
			
			Timer::stopTimer("updateLoop");	
		}
		
			
			
		//render
		
		if (SDL_GetTicks() - frameStart >= frameDelay) //if the time since the last frame is greater or equal to desired frame delay
		{
			Timer::startTimer("renderLoop");
			frameStart = SDL_GetTicks();
			GameTime::updateDeltaTime();
			this->render();
			
			Timer::stopTimer("renderLoop");
		}
		

		//clean memory fragmentation every 30 mins (1000ms *60 *30 = 1800000
		if (SDL_GetTicks() - defragStart >= 1800000) {
			entityManager.resize();
			_heapmin();
			defragStart = SDL_GetTicks();
		}
	}
	Timer::displayTimerStats("totalRuntime", -1);
	Timer::displayTimerStats("addingEntitie", -1);
	this->clean();
}


typedef std::vector<Entity*> EntityGroupV;
//FIX: change these to & instead of * at some point
EntityGroupV* tiles(&entityManager.getGroup(EntityGroup::groupMap));
EntityGroupV* items(&entityManager.getGroup(EntityGroup::groupItems));
EntityGroupV* players(&entityManager.getGroup(EntityGroup::groupPlayers));
EntityGroupV* enemies(&entityManager.getGroup(EntityGroup::groupEnemies));
EntityGroupV* projectiles(&entityManager.getGroup(EntityGroup::groupProjectiles));
EntityGroupV* weaponEffects(&entityManager.getGroup(EntityGroup::groupWeaponEffects));
EntityGroupV* colliders(&entityManager.getGroup(EntityGroup::groupColliders));
EntityGroupV* HUD(&entityManager.getGroup(EntityGroup::groupGameHUD));
EntityGroupV* overlay(&entityManager.getGroup(EntityGroup::groupDrawingOverlay));
EntityGroupV* gui(&entityManager.getGroup(EntityGroup::groupGUIMenu));
EntityGroupV* eventHandlers(&entityManager.getGroup(EntityGroup::groupInputHandlers));
EntityGroupV* structures(&entityManager.getGroup(EntityGroup::groupStructures));
EntityGroupV* interpolatables(&entityManager.getGroup(EntityGroup::groupInterpolatables));
EntityGroupV& gameEventHandlers(entityManager.getGroup(EntityGroup::groupEventHandlers));
void Game::handleSDLEvents()
{
	while (SDL_PollEvent(&event)) { //while has a queued event
		//regardless of event, check inputcontroller for response.
		//should be narrowed down only to keyboard/mouse inputs though, no input handler would deal with SDL_QUIT.

		bool buttonClicked = false;
		switch (event.type)
		{
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (Game::event.button.button) {
			case SDL_BUTTON_LEFT:
				for (Entity*& be : entityManager.getGroup(groupButtons)) {
					ButtonComponent* b = &be->getComponent<ButtonComponent>();
					SDL_Rect buttonArea{ static_cast<int>(b->tf->position.x),static_cast<int>(b->tf->position.y),b->tf->width,b->tf->height };
					if (Game::event.button.x >= buttonArea.x && Game::event.button.x <= buttonArea.x + buttonArea.w
						&& Game::event.button.y >= buttonArea.y && Game::event.button.y <= buttonArea.y + buttonArea.h) {
						b->clicked(true);
						buttonClicked = true;
					}
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			switch (Game::event.button.button) {
			case SDL_BUTTON_LEFT:
				for (Entity*& be : entityManager.getGroup(groupButtons)) {
					ButtonComponent* b = &be->getComponent<ButtonComponent>();
					SDL_Rect buttonArea{ static_cast<int>(b->tf->position.x),static_cast<int>(b->tf->position.y),b->tf->width,b->tf->height };
					if (Game::event.button.x >= buttonArea.x && Game::event.button.x <= buttonArea.x + buttonArea.w
						&& Game::event.button.y >= buttonArea.y && Game::event.button.y <= buttonArea.y + buttonArea.h) {
						b->clicked(false);
						buttonClicked = true;
					}
				}
			}
			break;
		case SDL_MOUSEWHEEL:
			//FIX: TEMP IMPLEMENTATION UNTIL CONTROL CAN BE USED NICELY
			//FIX: zoomBounds can be made const
			if (!Game::paused) {
				if (Game::event.wheel.preciseY < 0) Game::camera.zoom(-0.1f, { 0.5f,2.5f });
				else Game::camera.zoom(0.1f, { 0.5f,2.5f });
			}

		default:
			break;
		}
		//FIX: when updating inputControllers to use event details as well, this whole thing should just be for eventHandler: handleEvent();
		if (!buttonClicked) for (Entity*& c : *eventHandlers) c->getComponent<InputController>().eventOccuredResponse(event);
	}
}

void Game::handleGameEvents() {
	
	while (WHE_PollEvent(&gevent)) {

		//if nothing waiting to process events, clear events and skip process.
		if (gameEventHandlers.size() == 0) {
			WHE_ClearEvents();
			continue;
		}

		//iterate through handlers and do modifier responses. Modifier responses may invalidate or alter the events data.
		std::size_t i = 0;
		while (!gevent.isDeprecated() && i < gameEventHandlers.size()) {
			//FIX: priority sorting for effects i.e. internal negate, reflect, 50% reduction and external negates, etc. (i.e. i have a 1 time use negate, friend has aura that negates: aura (external) should be used first, 
			gameEventHandlers[i++]->getComponent<GameEventListenerComponent>().eventOccuredModifierResponse(gevent);
		}

		if (gevent.isDeprecated()) continue;

		//iterate through handlers and do listener responses. Listener responses do not alter the data (i.e. heal for dmg dealt, listen for amount of dmg dealt)
		i = 0;
		while (!gevent.isDeprecated() && i < gameEventHandlers.size()) {
			gameEventHandlers[i++]->getComponent<GameEventListenerComponent>().eventOccuredListenerResponse(gevent);
		}

		finaliseEvent(gevent);
	}
}
//other groups do collisionmethod on group1
extern void collideGroups(EntityGroupV& group1, std::vector<EntityGroupV*> otherGroups) {
	for (Entity*& e : group1) {
		for (std::vector<Entity*>*& group : otherGroups) {
			for (Entity*& otherE : *group) {
				if (CollisionHandler::doCollide(e->getComponent<ColliderComponent>().getHitbox(), otherE->getComponent<ColliderComponent>().getHitbox())) {
					CollisionHandler::processCollision(e, otherE);
				}
			}
		}
	}
}

//FIX: implement quad tree / world grid for checking collisions;
//		then implement everything can collide with everything else.
//	FIX: decide on how to handle everything collisions. im thinking of map of <EntityGroup, CollisionMethod> in each collider component
void Game::handleCollisions() {
	for (std::size_t i = 0; i < colliders->size()-1; ++i) {
		ColliderComponent& A = colliders->at(i)->getComponent<ColliderComponent>();
		for (std::size_t j = i+1; j < colliders->size(); ++j) {
			ColliderComponent& B = colliders->at(i)->getComponent<ColliderComponent>();

			if (CollisionHandler::canCollide(A, B)
				&& CollisionHandler::doCollide(A.getHitbox(), B.getHitbox()))
				
				CollisionHandler::processCollision(A, B);
		}
	}
}

Uint32 updateCount{ 0 };
Uint32 frameCount{ 0 };
Uint32 lastCalc{ 0 };
Uint32 lastBucketResort{ 0 };

void Game::update()
{
	if (!Game::paused) {
		Timer::startTimer("entityUpdate");
		entityManager.update(); //update causes things to be marked for deletion
		Timer::stopTimer("entityUpdate");
		
		Timer::startTimer("handleGEvents");
		handleGameEvents();
		Timer::stopTimer("handleGEvents");

		Timer::startTimer("entityRefresh");
		entityManager.refresh(); //delete things that need deletion
		Timer::stopTimer("entityRefresh");

		Timer::startTimer("handleCollisions");
		handleCollisions();
		Timer::stopTimer("handleCollisions");

		if (SDL_GetTicks() - lastCalc > 1000) {
			std::cout << frameCount / ((SDL_GetTicks() - lastCalc)/1000)<<"fps \n";
			frameCount = 0;
			lastCalc = SDL_GetTicks();
		}

	}
	else {
		for (Entity* e : *gui) {
			e->fixedUpdate(false);
		}
		if (currentScene->systemController != nullptr) currentScene->systemController->fixedUpdate(false);
	}
#pragma endregion
}

void Game::render()
{
	++frameCount;
	Timer::startTimer("SDL_RenderClear");
	SDL_RenderClear(renderer);
	Timer::stopTimer("SDL_RenderClear");

	Timer::startTimer("cameraCentering");
	if (!Game::paused) {
		if (players->size() > 0) Game::camera.centerOn((*players)[0]->getComponent<SpatialComponent>().getCenter());
		Game::camera.bind(currentScene->map.getBounds());
	}
	Timer::stopTimer("cameraCentering");
	//FIX: add code to sort out interpolation a bit better.
	// interpolatable objects should be decided by which entities are close enough to the camera as well
	// fix interpolation to be handled by buckets
	Timer::startTimer("interpolations");
	for (Entity*& e : *interpolatables) e->interUpdate();
	Timer::stopTimer("interpolations");

	Timer::startTimer("rendering");
	//get group in bucket (separate instances of Entity* from raw groups)
	std::array<std::vector<Entity*>, MAX_GROUPS>& groups = entityManager.getRenderables();
	//for each drawGroup
	for (std::size_t i = DRAW_GROUPS_START; i <= DRAW_GROUPS_END; ++i) {
		//get instance of group in this bucket
		std::vector<Entity*>& group{ groups[i]};
		for (Entity*& e : group) {
			e->draw();
		}
	}
	Timer::stopTimer("rendering");

	Timer::startTimer("interpolatedCollisions");
	collideGroups(*players, { colliders });
	collideGroups(*enemies, { colliders });
	Timer::stopTimer("interpolatedCollisions");

	Timer::startTimer("SDL_RenderPresent");
	SDL_SetRenderDrawColor(Game::renderer, 255, 255, 255, 255);
	SDL_RenderPresent(renderer);
	Timer::stopTimer("SDL_RenderPresent");

}

void Game::clean()
{
//	entityManager.nukeEntities();

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Game::loadScene(std::string sceneName)
{
	//unload current scene first
	//FIX: this means clear the entities and map

	//then load the new scene
	currentScene = assetManager->getScene(sceneName);
	currentScene->loadScene(entityManager);
}