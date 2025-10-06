#include "Src/Game Management/Game.hpp"
#include <chrono>
#include <thread>
#include "Src/Utils/Vector2D.hpp"
Game* game = nullptr;

int main(int argc, char* argv[])
{
	const int tileW = 32;
	const int tileH = 32;

	game = new Game();
	game->init("GameWindow", tileW * 24, tileH * 24, false, 20); //updates per second last param
	game->run(240); //fps as parameter
	return 0;
}