#include "EnemySpawnManager.hpp"
#include "Src/Game Management/Scene.hpp"
void EnemySpawnManager::spawnEnemies(int count) {
	std::vector<Vector2D*> nodes;
	//for each grass tile
	std::for_each(entityManager.getGroup(EntityGroup::groupMap).begin(), entityManager.getGroup(EntityGroup::groupMap).end(), [&nodes](Entity*& e) {if (e->getComponent<TileComponent>().srcRect.x == 0 && e->getComponent<TileComponent>().srcRect.y == 0) { nodes.push_back(&e->getComponent<SpatialComponent>().position); }});
	Vector2D* node = nullptr;
	int i = 0;
	while (enemiesSpawned.size() < enemyCap && ++i <= count) {
		node = nodes[rand() % nodes.size()];
		if (entityManager.getGroup(EntityGroup::groupPlayers).size() > 0 && node->distance(entityManager.getGroup(EntityGroup::groupPlayers)[0]->getComponent<SpatialComponent>().getCenter()) < 200) continue;
		entityManager.queueAddEntity<EnemyEntity>(*node);
		//enemies.back()->setAI(Game::behaviourManager->getObject("stayAtRangeAndShoot"));
		//Action* a{ enemies.back()->actions["Primary"] };
		//a->actionVariables["attackDelayMs"] = Uint32{ 1000 };
	}
	lastSpawn = GameTime::totalTime;
}