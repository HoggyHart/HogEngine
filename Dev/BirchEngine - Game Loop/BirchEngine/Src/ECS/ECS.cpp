#include "ECS.hpp"
#include "SpatialComponent.hpp"
#include "TileComponent.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Gameplay/Game Systems/CollisionHandler.hpp"
#include "SDL.h"
#include <iostream>
#include "Src/Utils/utils_SDL.hpp"
void Entity::addGroup(Group mGroup) {
	if (!this->hasGroup(mGroup)) {
		groupBitSet[mGroup] = true;
		entityManager.addToGroup(this, mGroup);
	}
}
#define ENTITY_RENDER_CULLING false
void Entity::draw() {
	//if / else if to cull any off-screen entities.
	//this all almost works. camera.x changes depending on zoom level, in that it becomes higher, i.e. zoom in will bring camera.x from 1000 to 2000, when positions of everything else doesnt change.
	//I recommend what i planned to do before, where camera keeps track of actual position AND its relative in-game position.
	//
	//to simplify the few instances of TF component vs Tile component, maybe have some sort of "Position" object they both inherit from, then can do a hasPosition() cehck on an entity.
	//
	//FOR WHATEVER REASON CULLING IS SLOWER THAN DRAWING EVERYTHING WITH THE CURRENT PROGRAM

	if (this->gameEntity) {
		for(std::unique_ptr<EComponent>& c : components) c->draw();
	}
	else {
		this->draw(nullptr);
	}
}

void Entity::draw(ViewPort* camera)
{
	for (auto& c : components) c->draw(camera);
}

void Manager::reSortBuckets()
{
	bucketEntities.refresh();
}