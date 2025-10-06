#pragma once

#include <vector>
#include <iostream>
#include <array>
#include <memory>
#include <algorithm>
#include <bitset>
#include <thread>
#include "GCST.hpp"
#include "Src/Utils/utils.hpp"
#include "Src/Gameplay/Game Systems/EntityGroups.hpp"

#include "Src/Engine/Buckets/RenderBuckets.hpp"
class EComponent;
class Entity;
class Manager;

using ComponentArray = std::array<EComponent*, maxComponents>;

class EComponent : public Component<Entity> {
public:
	virtual void init() {}
	virtual void fixedUpdate(bool endOfInterpolation) {}
	virtual void interUpdate() {};
	virtual void draw() {}
	virtual void draw(ViewPort* camera) {} // for choosing which camera to draw to
	EComponent(Entity& e) : Component<Entity>(e) {};

	virtual ~EComponent() {}
};

class Entity : public ComponentSystem<EComponent, Entity>, public Unique_ID_Object {
protected:

	bool active = true; //still in use
	bool gameEntity = true;
	//groups
	GroupBitSet groupBitSet = {};

	//dependant entities, so if this entity is destroyed, these ones are destroyed alongside it
	std::vector<Entity*> dependantEntities;
public:
	Manager& entityManager;
#pragma region StandardEntityStuff
	Entity(Manager& memberManager) : Unique_ID_Object(), entityManager(memberManager) {
	}
	Entity(Entity& e) : Unique_ID_Object(), entityManager(e.entityManager) {
		//loop through array instead of vector as it preserves the order the components were added in

		for (EComponent*& c : componentArray) this->addComponent(c);
	}

	virtual ~Entity() {
	};
	virtual void fixedUpdate(bool EOI) { for(auto& c : components) c->fixedUpdate(EOI); }
	virtual void interUpdate() { for (auto& c : components) c->interUpdate(); }
	virtual void draw();
	void makeDisplayEntity() { gameEntity = false; }
	void makeGameEntity() { gameEntity = true; }
	bool isGameEntity() const { return gameEntity; }
	
	
	virtual void draw(ViewPort* camera); //for giving a specific camera to draw to (i.e. draw in-game or drwa to window)
	bool isActive() const { return active; }
	void destroy() {
		active = false;
		for (Entity*& e : dependantEntities) {
			if (e->isActive()) e->destroy();
		}
	}
	void addDependant(Entity* e) { dependantEntities.push_back(e); }
	bool hasGroup(Group memberGroup) {
		return groupBitSet[memberGroup];
	}
	GroupBitSet getGroups() {
		return groupBitSet;
	}
	void addGroup(Group newGroup);
	void delGroup(Group oldGroup) {
		groupBitSet[oldGroup] = false;
	}
	void clearGroups() {
		for (int i = 0; i < maxGroups; ++i) {
			groupBitSet[i] = false;
		}
	}
#pragma endregion
};


//handles updating and drawing all entities in the game
class Manager {
private:
	//unique ptrs to delete entity when needed.
	//could use regular pointers as long as they are deleted in EntityManager::refresh, as thats when they are out of use.
//	std::vector<Entity*> entities;
	//entities must be queued to be added so that the entities iterator doesnt get invalidated when an entity is created in an update method
	std::vector<Entity*> queuedEntities;

	//FIX: add some moveToBackOfGroup stuff? to sort layers? i.e. if i have 2 windows and i want to bring one to the front, it needs to be pushed to the back of its drawGroup so that it gets drawn on top
	//std::array<std::vector<Entity*>, MAX_GROUPS> groupedEntities;

	//buckets
	BucketManager bucketEntities{ {500,1500,5000,30000},{3,7,11} };
	bool paused;
public:
	~Manager() {
		nukeEntities();
	}
	void update() {
		bucketEntities.update();
	}

	void refresh() {
		//i THINK it HAS to go
		//1. queue -> entities : doesnt really matter i dont think
		//2. groups -> clear dead : if inactive are deleted from entities first, the pointers here will become dangling ones
		//3. entities -> clear dead : same as number 2.

		//add queued entities (cannot be added during update())
		for (Entity*& e : queuedEntities) {
			//entities.push_back(e);
			bucketEntities.buckets[0].push_back(e);
		}
		queuedEntities.clear();

		bucketEntities.refresh();

	}
	void reSortBuckets();
	void resize() {
		bucketEntities.shrinkwrap();
	}
	
	void nukeEntities() {
		bucketEntities.destroyAll();
		for (auto& e : queuedEntities) e->destroy();
		refresh();
	}
	void addToGroup(Entity* mEntity, Group mGroup) {
		bucketEntities.addEntityToGroup(mEntity, mGroup);
	}
	std::vector<Entity*>& getGroup(Group mGroup) {
		return bucketEntities.groupBuckets[0][mGroup];
	}
	std::array<std::vector<Entity*>,MAX_GROUPS>& getRenderables() {
		return bucketEntities.groupBuckets[0];
	}
	std::vector<Entity*>& getEntities() {
		return bucketEntities.buckets[0];
	}

	template<typename CustomEntity, typename ...EntityArgs>
	CustomEntity& addEntity(EntityArgs ...args) {
		Entity* e = new CustomEntity(*this, args...);
	//	entities.push_back(e);
		bucketEntities.buckets[0].push_back(e);
		return *static_cast<CustomEntity*>(e);
	}
	template<typename CustomEntity, typename ...EntityArgs>
	CustomEntity& queueAddEntity(EntityArgs ...args) {
		Entity* e = new CustomEntity(*this, args...);
		queuedEntities.emplace_back(e);
		return *static_cast<CustomEntity*>(e);
	}
};