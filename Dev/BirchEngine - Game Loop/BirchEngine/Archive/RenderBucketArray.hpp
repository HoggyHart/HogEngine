#pragma once
#include <array>
#include <vector>
#include <SDL.h>
#include "Src/ECS/SpatialComponent.hpp"
//first = last refresh, second = refresh delay
typedef std::pair<unsigned int, unsigned int> RefreshTimer;


class Entity;
template<const std::size_t maxGroups, const std::size_t maxBuckets>
struct RenderBucketManager {
	typedef std::array<std::vector<Entity*>, maxGroups> EntityGroups;
	typedef std::vector<Entity*> EntityGroup;

	//Contains a LastRefresh and RefreshDelay value for each bucket
	std::array<RefreshTimer, maxBuckets> bucketRefreshRates;
	//Contains a camera scale multiplier to determine which bucket entities go in based on distance from the camera
	std::array<Uint32, maxBuckets> bucketSizes;
	//Contains entities in each bucket/array index
	//specifically contains them in the same format as EntityGroups to make it easy to add entities into the right part of the bucket so they are drawn on the correct layer
	std::array <EntityGroups, maxBuckets> groupBuckets;

	//pass in an array containing the refresh delay of bucket 1,2,etc. in ms
	RenderBucketManager(std::array<std::size_t, maxBuckets> refreshRates, std::array<Uint32, maxBuckets> bucketSizes);

	void refresh();

	void sortBucket(EntityGroups& bucket);

};

template<std::size_t maxGroups, std::size_t maxBuckets>
inline RenderBucketManager<maxGroups, maxBuckets>::RenderBucketManager(std::array<std::size_t, maxBuckets> refreshRates, std::array<Uint32, maxBuckets> bucketSizes) : bucketSizes{ bucketSizes }
{
	for (std::size_t i = 0; i < maxBuckets; ++i) {
		bucketRefreshRates[i].first = 0u;
		bucketRefreshRates[i].second = refreshRates[i];
	}
}

template<std::size_t maxGroups, std::size_t maxBuckets>
inline void RenderBucketManager<maxGroups, maxBuckets>::refresh()
{
	for (std::size_t i = 0; i < maxBuckets; ++i) {
		if (SDL_GetTicks() - bucketRefreshRates[i].first >= bucketRefreshRates[i].second) {
			sortBucket(groupBuckets[i]);
		}
	}
}

template<std::size_t maxGroups, std::size_t maxBuckets>
inline void RenderBucketManager<maxGroups, maxBuckets>::sortBucket(EntityGroups& bucket)
{
	for (std::size_t g = 0; g <= maxGroups; ++g) {
		std::vector<Entity*>& group = bucket[g];
		//for each entity in drawgroup
		for (Entity*& e : group) {
			//if is a display entity, add to bucket 1 since it's always visible
			if (!e->isGameEntity()) renderBucket[g].push_back(e);
			else {
				//if entity is within bucket 1 area, add to bucket 1
				if (CollisionHandler::rectCollision(e->getComponent<SpatialComponent>().toRect(), bucket1Area))
					renderBucket[g].push_back(e);
				//repeat for buckets 2/3/etc.
				//then add to last bucket if wasnt in range of defined buckets
				else
					renderBucket.back().push_back(e);
			}
		}
	}
}
