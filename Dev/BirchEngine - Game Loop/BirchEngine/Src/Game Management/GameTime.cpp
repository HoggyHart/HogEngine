#include "GameTime.hpp"

Uint32 GameTime::lastUpdate{ 0 };
float GameTime::timeSpeedMult{ 1.0f };
Uint32 GameTime::totalTime{ 0 };
bool GameTime::timeFrozen{ false };
Uint32 GameTime::fractionalDeltaTime{ 0 };
Uint32 GameTime::accumulatedDeltaTime{ 0 };
Uint32 GameTime::deltaTime{ 0 };
Uint32 GameTime::fixedDeltaTime{ 0 };
