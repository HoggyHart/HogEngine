#pragma once
#include "Src/Gameplay/CreatureStuff/Instances/Enemy.hpp"
//FIX: check this works + also check out GLOBAL entity limits + entity despawning + etc.
// i.e. generally create a more secure entity spawning system
class EnemySpawnManager {
private:
	Manager& entityManager;
	Uint32 spawnDelay;
	Uint32 lastSpawn{ 0u };

	Uint32 enemyCap;
	std::vector<EnemyEntity*> enemiesSpawned;
public:
	EnemySpawnManager(Manager& scene, Uint32 enemyCap, Uint32 spawnDelay) : entityManager(scene), enemyCap(enemyCap), spawnDelay(spawnDelay) {}
	void setSpawnSettings(Uint32 cap, Uint32 delay) { enemyCap = cap; spawnDelay = delay; };

	void fixedUpdate() {
		enemiesSpawned.erase(
			std::remove_if(
				enemiesSpawned.begin(),
				enemiesSpawned.end(),
				[](EnemyEntity*& enemy) {
					return !enemy->isActive();
				}
			),
			enemiesSpawned.end()
		);

		if (GameTime::totalTime - lastSpawn > spawnDelay) {
			spawnEnemies(3);
		}
	}

	void spawnEnemies(int count);
};