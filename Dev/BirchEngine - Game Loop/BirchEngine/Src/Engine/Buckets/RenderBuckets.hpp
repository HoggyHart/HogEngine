#pragma once
#include <array>
#include <vector>
#include <bitset>
#include <SDL.h>
#include "Src/Engine/EntityManagementConstants.hpp"
#include "Src/Gameplay/Game Systems/EntityGroups.hpp"
//first = last refresh, second = refresh delay
typedef std::pair<unsigned int, unsigned int> RefreshTimer;

//used with bucket sizes to prevent underflow i.e. bucketX = x - w/2 -w*bucketScale/2 going negative
//can be easily rmeoved. and should be FIX:
struct SignedSDL_Rect {
	long x;
	long y;
	long w;
	long h;
	SignedSDL_Rect(SDL_Rect r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
	SignedSDL_Rect(long x, long y, long w, long h) : x(x), y(y), w(w), h(h) {}
};

class Entity;
typedef std::vector<Entity*> EntityGroupV;
typedef std::vector<Entity*> Bucket;
typedef std::array<EntityGroupV,MAX_GROUPS> GroupBucket;


struct BucketManager{
	
	
	
	//Contains a LastRefresh and RefreshDelay value for each bucket
	std::array<RefreshTimer,MAX_BUCKETS> bucketRefreshRates;
	//Contains a camera scale multiplier to determine which bucket entities go in based on distance from the camera
	std::array<float,MAX_BUCKETS-1> bucketScales; //defined on initialisation of bucketmanager
	std::array<SDL_Rect,MAX_BUCKETS-1> bucketAreas; //updated every refresh() cycle


	std::array<Bucket, MAX_BUCKETS> buckets{};
	//Contains entities in each bucket/array index
	//specifically contains them in the same format as EntityGroups to make it easy to handle drawing entities on the right layer
	std::array<GroupBucket, MAX_BUCKETS> groupBuckets{};

	//pass in an array containing the refresh delay of bucket 1,2,etc. in ms
	//FIX: bucketSizes should be -1 to size, since last bucket is just "everything else". Should appropriately change any index accessing loops of bucketSizes to account for this
	BucketManager(std::array<std::size_t, MAX_BUCKETS> refreshRates, std::array<float,MAX_BUCKETS-1> bucketSizes);
	
	inline void recalculateBucketAreas();
	void update();
	void refresh();
	void shrinkwrap();
	void destroyAll();
	//dont think groupSizes is needed anymore since not sorting by groupBuckets
	void sortBucket(std::size_t bucketIndex, std::size_t& originalEntityCount);
	void cleanBucket(std::size_t bucketIndex);

	void addEntityToGroup(Entity*& e, std::size_t& group);
	//returns bool to indicate if this entity was REMOVED from the bucket indicated by currentBucketIndex (if not nullptr)
	bool sortEntityIntoBucket(Entity*& e, std::size_t* currentBucket, std::size_t* originalEntityCount);
	bool moveEntityIntoBucket(Entity*& e, std::bitset<MAX_GROUPS> entityGroups, std::size_t goalBucket, std::size_t*& currentBucketIndex, std::size_t* originalEntityCount);
	void removeEntityFromBuckets(Entity*& e, std::size_t* bucketIndex, std::size_t* originalEntityCount);
	bool removeFromBucket(Entity*& e, std::size_t bucketIndex, std::size_t* originalEntityCount);

	
};