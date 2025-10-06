#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <vector>
#include <functional>
#include <mutex>
#include "Src/Utils/Camera.hpp"
#include "Src/Gameplay/Game Systems/GameEvents.hpp"
#define DEBUG(x) std::cout<<#x<<": "<<x<<"\n"
#define BASE_GAME_MS_PASSED_PER_TICK 50
class StaticGameDataManager;
class Game
{
public:
	Game();
	~Game();

	void init(const char* title, int width, int height, bool fullscreen, const int TPS);
	//void run();

	static int TPS;
	static bool paused;
	void run(unsigned int FPS);
	void handleSDLEvents();
	void handleGameEvents();
	void handleCollisions();
	void update();
	bool running() { return isRunning; }
	void render();
	void clean();

	static SDL_Renderer* renderer;
	static SDL_Event event;
	static WHE_Event gevent;
	static bool isRunning;
	static ViewPort camera;

	static void loadScene(std::string sceneName);

	static StaticGameDataManager* assetManager;
	static bool displayHitboxes;
private:
	static Uint32 gameTimePerTick;
	static Uint32 gameTime;
	float gameSpeedMult{ 1.0f };
	SDL_Window* window = nullptr;
};