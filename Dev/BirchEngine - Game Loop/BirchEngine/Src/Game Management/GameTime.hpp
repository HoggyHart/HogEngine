#pragma once
#include <SDL.h>
#include <math.h>
struct GameTime {
private:
	static Uint32 lastUpdate;
	
public:
	static Uint32 totalTime;
	static float timeSpeedMult;
	static bool timeFrozen;
	static Uint32 accumulatedDeltaTime;
	
	static Uint32 fractionalDeltaTime; //for storing real time stored to add up to deltaTime
	// i.e. if updateDeltaTime() is called every 1ms then lastUpdate will always be 1 less than SDL_GetTicks() result
	// and if timeSpeedMult is 0.5 then deltaTime becomes 0 since (X - (X-1)) * timeSpeed = 1 * 0.5 which gets truncated to 0
	// this means that instead of time being 0.5 speed it goes to 0 speed

	/*
	Example:
		fdt =0; dt = 0; m = 0.5; tdt =0;

		_update1_
		fdt+=1; (fdt = 1)
		dt = fdt * m; (1*0.5 = 0)
		fdt = fdt - dt/m ( 1 - 0/0.5 = 1 - 0 = 1)
		
		_update2_
		fdt+=1; (fdt = 2)
		dt = fdt * m; (2*0.5 = 1)
		fdt = fdt - dt/m ( 2 - 1/0.5 = 2 - 2 = 0)

		THEN THE CYCLE REPEATS
	*/

	static Uint32 deltaTime;
	static Uint32 fixedDeltaTime;

	static void updateDeltaTime(){ 
		Uint32 realTimePassed = SDL_GetTicks() - lastUpdate;
		fractionalDeltaTime += realTimePassed;

		deltaTime = fractionalDeltaTime * timeSpeedMult * !timeFrozen;
		fractionalDeltaTime = fractionalDeltaTime - deltaTime / timeSpeedMult;
		lastUpdate = SDL_GetTicks();
		//limit deltaTime so that the max time that can pass between update ticks is fixedDeltaTime
		//	accumulatedDeltaTime is the total amount of deltaTime passed since the last resetAccumulated() call that happens when update() is called
		if (deltaTime + accumulatedDeltaTime > fixedDeltaTime) { //
			deltaTime = fixedDeltaTime - accumulatedDeltaTime;
		}
		
		accumulatedDeltaTime += deltaTime;
		totalTime += deltaTime;
	}
	static void setFixedDeltaTime(Uint32 fdt) { fixedDeltaTime = fdt; }
	static void resetAccumulated() { accumulatedDeltaTime = 0; }

};