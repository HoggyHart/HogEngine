#include "Map.hpp"
#include <fstream>
#include "Src/Gameplay/WorldStuff/TileEntity.hpp"
unsigned __int64 a;
unsigned __int64 b;
int tsSize;
Map::Map(std::string mapTilesAssetID, float mapScale, int ts) : tilesAssetID(mapTilesAssetID), scale(mapScale), tilesize(ts) {
	scaledTileSize = static_cast<int>(mapScale * ts);
}
Map::~Map() {
}
void tempDisplayRects(std::vector< std::pair<SDL_Rect, bool>> rects);
void Map::LoadMap(const char* mapInfo, int sizeX, int sizeY, Manager* man) {
	mapBounds = SDL_Rect{ 0,0,sizeX * scaledTileSize,sizeY * scaledTileSize };
	tsSize = scaledTileSize;
	char tile;
	std::fstream mapFile;
	mapFile.open(mapInfo);
	int srcX, srcY;
	std::vector<TileEntity*> tiles;
	for (int y = 0; y < sizeY; ++y) {
		for (int x = 0; x < sizeX; ++x) {
			mapFile.get(tile); //get next char
			srcY = atoi(&tile) * this->tilesize; //convert char to tile X position in spriteSheet
			mapFile.get(tile);
			srcX = atoi(&tile) * this->tilesize; //same for Y
			tiles.push_back(addTile(srcX, srcY, x * scaledTileSize, y * scaledTileSize, man)); //change to tiles have colliders, rather than colliders on seperate layer
			mapFile.ignore(); //ignores comma
		}
	}
	//now go to next layer to check hitboxes
	mapFile.ignore();
	::a = sizeY;
	::b = sizeX;
	std::vector <std::pair<SDL_Rect, bool>> collisionTiles;
	int i = 0;
	for (int y = 0; y < sizeY; ++y) {
		for (int x = 0; x < sizeX; ++x) {
			mapFile.get(tile);
			if (tile != '-') {
				//tiles[i]->makeCollidable("terrain");
				collisionTiles.push_back({ tiles[i]->getComponent<SpatialComponent>().toRect(),true });
			}
			++i;
			mapFile.ignore();
			mapFile.ignore();
		}
	}
	bool fusedABox = true;
	if (collisionTiles.size() > 1) {
		while (fusedABox) {
			fusedABox = false;
			//fuse horizontally
			std::size_t cbi = 0; //current box index i.e. current one being looked at
			std::size_t nbi = 1;
			//fuse vertically
			while (cbi < collisionTiles.size() - 1) {
				if (collisionTiles[cbi].second) {
					while (nbi < collisionTiles.size()) {
						if (collisionTiles[nbi].second) {
							if (collisionTiles[cbi].first.y == collisionTiles[nbi].first.y
								&& collisionTiles[cbi].first.h == collisionTiles[nbi].first.h
								&& collisionTiles[cbi].first.x + collisionTiles[cbi].first.w == collisionTiles[nbi].first.x)
							{
								collisionTiles[cbi].first.w += collisionTiles[nbi].first.w; //fuse this and the next rect
								collisionTiles[nbi].second = false; //then set to destroy the next rect
								fusedABox = true;
							}
						}
						++nbi;
					}
				}
				++cbi;
				nbi = cbi + 1;
			}
			cbi = 0; //current box index i.e. current one being looked at
			nbi = 1;
			while (cbi < collisionTiles.size() - 1) {
				if (collisionTiles[cbi].second) {
					while (nbi < collisionTiles.size()) {
						if (collisionTiles[nbi].second) {
							if (collisionTiles[cbi].first.x == collisionTiles[nbi].first.x
								&& collisionTiles[cbi].first.w == collisionTiles[nbi].first.w
								&& collisionTiles[cbi].first.y + collisionTiles[cbi].first.h == collisionTiles[nbi].first.y)
							{
								collisionTiles[cbi].first.h += collisionTiles[nbi].first.h; //fuse this and the next rect
								collisionTiles[nbi].second = false; //then set to destroy the next rect
								fusedABox = true;
							}
						}
						++nbi;
					}
				}
				++cbi;
				nbi = cbi + 1;
			}

			collisionTiles.erase(std::remove_if(collisionTiles.begin(), collisionTiles.end(), [](std::pair<SDL_Rect, bool>& p) {return p.second == false; }), collisionTiles.end());
			//tempDisplayRects(collisionTiles);
		}

		for (auto& p : collisionTiles) {
			Entity* e = &man->addEntity<Entity>();
			e->addComponent<SpatialComponent>(static_cast<float>(p.first.x), static_cast<float>(p.first.y), p.first.w, p.first.h, 1.0f);
			ColliderComponent& c = e->addComponent<ColliderComponent>(HitboxShape::Rectangle, 1u);
			e->addGroup(EntityGroup::groupDrawingOverlay);
			e->addGroup(EntityGroup::groupColliders);
		}
	}
	mapFile.close();
}
void tempDisplayRects(std::vector< std::pair<SDL_Rect, bool>> rects) {
	std::vector<std::vector<std::size_t>> hbg{ a, std::vector<std::size_t>(b,0) };
	int i = 1;
	for (auto& p : rects) {
		for (int h = 0; h < p.first.h / tsSize; ++h) {
			for (int w = 0; w < p.first.w / tsSize; ++w) {
				hbg[p.first.y / tsSize + h][p.first.x / tsSize + w] = i;
			}
		}
		i += 1;
	}
	for (auto& v : hbg) {
		for (auto& val : v) {
			std::cout << val << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}
TileEntity* Map::addTile(int srcX, int srcY, int x, int y, Manager* man) {
	return &man->addEntity<TileEntity>(srcX, srcY, x, y, this->tilesize, this->scale, tilesAssetID);
}