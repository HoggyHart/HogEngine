#pragma once
#include "RenderBuckets.hpp"
#include "Src/ECS/SpatialComponent.hpp"
#include "Src/Gameplay/Game Systems/CollisionHandler.hpp"
#include "Src/Utils/Timer.hpp"
BucketManager::BucketManager(std::array<std::size_t, MAX_BUCKETS> refreshRates, std::array<float, MAX_BUCKETS - 1> bucketSizes)
	:
	//X amount of EntityGroups objects, containing Y amount of EntityGroup objects
	bucketRefreshRates{ RefreshTimer{0u,0u} },
	bucketScales{ bucketSizes },
	bucketAreas{ SDL_Rect{} }
{

	for (std::size_t i = 0; i < MAX_BUCKETS; ++i) {
		bucketRefreshRates[i].second = refreshRates[i];
	}
}
void BucketManager::addEntityToGroup(Entity*& e, std::size_t& group)
{
	//FIX:
	// entity should be added to groups independently of main entity bucket
	// this means when an entity is added in ECS.hpp with addEntity() it needs to
	// A) not create duplicates
	// B) maybe not even affect groups
	// C) idk

	//if entity is found in a bucket, add it to that bucket's groups
	for (std::size_t i = 0; i < buckets.size();++i) {
		if (std::find(buckets[i].begin(), buckets[i].end(), e) != buckets[i].end()) {
			groupBuckets[i][group].push_back(e);
			return;
		}
	}

	//if entity not in a bucket yet, add it to the first groupBucket.
	//this will/should only happen if the entity is still being initialised
	groupBuckets[0][group].push_back(e);
	//we do NOT add it the unsorted bucket yet because that gets done after the entity is finished initialising, also put into the first bucket

	//this is done because some entities may have no groups and will only be added to the buckets at all in the addEntity() method
	//to prevent needing to check if the entitiy is already in the bucket at this point, we just make sure it cant be.
	//then if bucket[0] is not the right place for it, it gets sorted out next refresh cycle

}
void BucketManager::recalculateBucketAreas() {
	//recalculate bucket sizes since camera may have moved
	SignedSDL_Rect cam{ Game::camera }; //camera rect is in int form so that x and y positions for bucketAreas can go negative and therefore bounded if they do so
	//if still in Uint32 form they will underflow to 4 billion or so, making it more complicated to bind

	//for each area: recalculate area
	for (std::size_t i = 0; i < bucketAreas.size(); ++i) {
		float& scale = bucketScales[i]; //doesnt *need* to be float&, but I think its more memory efficient as opposed to making a copy with a float declaration
		SDL_Rect& area = bucketAreas[i];
		area.w = cam.w * scale;
		area.h = cam.h * scale;

		//if x would be out-of-bounds
		int posPart = cam.x + (cam.w - (cam.w * scale)) / 2;
		if (posPart < 0) {
			area.w += posPart;
			posPart = 0;
		}
		area.x = posPart;

		//same for y
		posPart = cam.y + (cam.h - (cam.h * scale)) / 2;
		if (posPart < 0) {
			area.h += posPart;
			posPart = 0;
		}
		area.y = posPart;
	}

}
void BucketManager::update()
{
	//MAKE BETTER
	//for each bucket
	for (Entity*& e : buckets[0]) e->fixedUpdate(true);
	for (std::size_t i = 1; i < buckets.size(); ++i) {
		//if refresh delay is finished, update.
		if (SDL_GetTicks() - bucketRefreshRates[i].first >= bucketRefreshRates[i].second) {
			for (Entity*& e : buckets[i]) e->fixedUpdate(false);
		}
	}
}

void BucketManager::refresh()
{
	recalculateBucketAreas();

	Timer::startTimer("bucketCleaning");
	std::array<std::size_t, MAX_BUCKETS> originalBucketEntityCounts{};
	//record pre-refresh group sizes
	for (std::size_t b = 0; b < buckets.size(); ++b) {
		//prune all inactive entities /de-grouped entities
		cleanBucket(b);
		originalBucketEntityCounts[b] = buckets[b].size();
	}
	Timer::stopTimer("bucketCleaning");

	Timer::startTimer("bucketSorting");
	//for each bucket
	for (std::size_t i = 0; i < buckets.size(); ++i) {
		
		//if time to sort, do so
		if (SDL_GetTicks() - bucketRefreshRates[i].first >= bucketRefreshRates[i].second) {
			//if time to refresh, sort bucket's entities into right bucket and simultaneously remove any inactive ones
			sortBucket(i, originalBucketEntityCounts[i]);
			bucketRefreshRates[i].first = SDL_GetTicks();
		}
	}
	Timer::stopTimer("bucketSorting");
}

void BucketManager::shrinkwrap()
{
	for (std::size_t bucketIndex = 0u; bucketIndex < MAX_BUCKETS; ++bucketIndex) {
		buckets[bucketIndex].shrink_to_fit();
		for (std::vector<Entity*>& v : groupBuckets[bucketIndex]) {
			v.shrink_to_fit();
		}
	}
	
}

void BucketManager::destroyAll()
{
	for (std::size_t bucketIndex = 0u; bucketIndex < MAX_BUCKETS; ++bucketIndex) {
		for (Entity*& e : buckets[bucketIndex]) {
			e->destroy();
		}
	}
}

void BucketManager::cleanBucket(std::size_t bucketIndex)
{
	//remove inactive entities from unsorted bucket

	//go through groups to remove any inactive/de-grouped entities
	Timer::startTimer("cleaningBucketGroups");
	GroupBucket& bucket{ groupBuckets[bucketIndex] };
	for (std::size_t g = 0; g < 32; ++g) {
		std::vector<Entity*>& v = bucket[g];
		v.erase(std::remove_if(v.begin(), v.end(), [g](Entity*& mEntity) {
			return !mEntity->isActive() || !mEntity->hasGroup(g);
			}
		), v.end());
	}
	Timer::stopTimer("cleaningBucketGroups");

	Timer::startTimer("cleaningMainBuckets");
	Bucket& b{ buckets[bucketIndex] };
	b.erase(std::remove_if(b.begin(), b.end(), [](Entity*& e) {
		if (!e->isActive()) {
			delete e;
			return true;
		}return false;
		}), b.end());
	Timer::stopTimer("cleaningMainBuckets");
}
void BucketManager::sortBucket(std::size_t bucketIndex, std::size_t& originalEntityCount)
{
	//store bucket so it doesnt have to do index lookup for each entity
	Bucket& b{ buckets[bucketIndex] };
	std::size_t numToReSort = originalEntityCount;
	//for entity in bucket
	for (std::size_t eI = 0; eI < numToReSort; ++eI) {
		
		Entity* e = b[eI];
		int change = 0;
		//move entity to right bucket 
		//returns 1 if removed from this bucket (to move into another) or 0 if it was left in this one
		change = sortEntityIntoBucket(e, &bucketIndex, &originalEntityCount);
		//change will be 0 if not moved, and 1 if removed from this bucket (either to delete or to move bucket)
		numToReSort -= change;
		eI -= change;
	}
}


bool BucketManager::sortEntityIntoBucket(Entity*& e, std::size_t* currentBucketIndex, std::size_t* originalEntityCount)
{
	//if display entity add to bucket 1 to render asap since its probably supposed to always be on screen and constantly rendered
	if (!e->hasComponent<SpatialComponent>() || !e->isGameEntity()) {
		return moveEntityIntoBucket(e, e->getGroups(), 0, currentBucketIndex,originalEntityCount);
	}


	SDL_Rect entitySpace = e->getComponent<SpatialComponent>().toRect();
	//test each specifically designed bucket to see if this entity goes in it, smallest bucket area first
	for (std::size_t i = 0; i < bucketAreas.size(); ++i) {
		//if entity fits in this bucket
		if (CollisionHandler::rectCollision(entitySpace, bucketAreas[i])) {
			return moveEntityIntoBucket(e, e->getGroups(), i, currentBucketIndex, originalEntityCount);
		}
	}
	//if didnt fit into the defined bucket areas, pop it in the outermost bucket
	return moveEntityIntoBucket(e, e->getGroups(), groupBuckets.size() - 1, currentBucketIndex, originalEntityCount);
}

bool BucketManager::moveEntityIntoBucket(Entity*& e, std::bitset<MAX_GROUPS> entityGroups, std::size_t targetBucket, std::size_t*& currentBucket, std::size_t* originalEntityCount) {

	bool removed = false;
	//if entity is currently in a bucket
	if (currentBucket != nullptr) {
		//if moving into the bucket its already in, return false
		if (*currentBucket == targetBucket) return removed;

		//if it is moving into this bucket from another, remove it from the other bucket
		removeEntityFromBuckets(e, currentBucket,originalEntityCount);
		removed = true;
	}
	
	//add to unsorted bucket
	buckets[targetBucket].push_back(e);
	//add to grouped buckets
	for (std::size_t i = 0; i < entityGroups.size(); ++i) {
		//if not in group, go to next
		if (entityGroups[i] == 0) continue;
		//if in group, add to this part of the bucket
		else groupBuckets[targetBucket][i].push_back(e);
	}

	return removed;
}

bool BucketManager::removeFromBucket(Entity*& e, std::size_t bucketIndex, std::size_t* originalEntityCount) {
	
	bool removedFromBucket = false;
	EntityGroupV::iterator pos = std::find(buckets[bucketIndex].begin(), buckets[bucketIndex].end(), e);
	if (pos != buckets[bucketIndex].end()) {
	
		buckets[bucketIndex].erase(pos);
		//if this method was called from the reSortingBucket method, make sure it knows that there is 1 fewer entity in this bucket now
		if (originalEntityCount != nullptr) (*originalEntityCount) -= 1;
		//this could be done the same as before but instead we return true so the reSort/moveBucket call receives this true and knows that the entity was removed, so reduces the index by 1 to account for the shift of entities to fill the space
		removedFromBucket = true;
	}
	//if not found in unsorted bucket, wont be in grouped buckets so no need to search
	if (removedFromBucket == false) return false; 

	//then remove from grouped bucket
	//for each group
	for (std::size_t i = 0; i < groupBuckets[bucketIndex].size(); ++i) {
		EntityGroupV& g = groupBuckets[bucketIndex][i];
		//if entity found in group
		EntityGroupV::iterator pos = std::find(g.begin(), g.end(), e);
		if (pos != g.end()) {
			//remove it
			g.erase(pos);
		}
	}
	return true;
}

void BucketManager::removeEntityFromBuckets(Entity*& e, std::size_t* bucketIndex, std::size_t* originalEntityCount)
{
	//if unknown bucket location, search through buckets
	if (bucketIndex == nullptr) {
		for (std::size_t b = 0; b < groupBuckets.size(); ++b) {
			//try to remove from each bucket.
			//if found and removed from a bucket (method returned true), stop the search
			if (removeFromBucket(e, b, originalEntityCount)) return;
		}
	}
	else {
		removeFromBucket(e, *bucketIndex, originalEntityCount);
	}
}





