#pragma once
#include <string>
#include <SDL.h>
class Manager;
class TileEntity;
class Map {
public:
	Map() {};
	Map(std::string tilesAssetID, float mapScale, int tileSize);
	~Map();

	void LoadMap(const char* mapInfoFile, int sizeX, int sizeY, Manager* man);
	TileEntity* addTile(int srcX, int srcY, int x, int y, Manager* man);
	SDL_Rect getBounds() const { return mapBounds; }

private:
	SDL_Rect mapBounds{ -1,-1,-1,-1 };
	std::string tilesAssetID{ "" };
	float scale{ 0.0f };
	int tilesize{ 0 };
	int scaledTileSize{ 0 };
};