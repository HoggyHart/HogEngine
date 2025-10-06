#pragma once

#define DRAW_GROUPS_START groupMap
#define DRAW_GROUPS_END groupGUIMenu

typedef std::size_t EntityGroupIndex;
enum EntityGroup {
	//Drawn groups
	groupMap,
	groupStructures,
	groupItems,
	groupPlayers,
	groupProjectiles,
	groupWeaponEffects,
	groupEnemies,
	groupGameHUD,
	groupDrawingOverlay,
	groupGUIMenu,
	//Tag groups

	groupGlobalColliders, //FIX: temp group. not sure how to properly handle ALL collisions.
	//i.e. attacks usually just interact with Creatures i.e. groupEnemies or groupPlayers, but what if a special sweep attack should be able to destroy projectiles? i just add it to global colliders for now

	groupInterpolatables,
	groupColliders,
	groupButtons,
	groupWindows, //for keeping track of certain windows
	groupInputHandlers,
	groupEventHandlers,
	groupInteractables
};