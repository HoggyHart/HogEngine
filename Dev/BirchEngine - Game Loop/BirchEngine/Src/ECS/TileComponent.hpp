#pragma once
#include "SDL.h"
#include <string>
#include "Src/Game Management/ObjectManagement.hpp"
#include "Src/Game Management/ObjectManagers/AssetManager.hpp"
#include "Src/Game Management/TextureManager.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Game Management/Game.hpp"
#include "SpatialComponent.hpp"

class TileComponent : public EComponent {
public:
	SpatialComponent* sc{ nullptr };
	SDL_Texture* tileTexture{ nullptr };
	SDL_Rect srcRect{}, dstRect{};

	TileComponent(Entity& e) : EComponent(e) {};
	TileComponent(Entity& e, int srcX, int srcY, int x, int y, int tsize, float tscale, std::string tileTexID) : EComponent(e),
		sc{ &e.getComponent<SpatialComponent>() },
		tileTexture{ Game::assetManager->getTexture(tileTexID) }
	{
		srcRect.x = static_cast<int>(srcX);
		srcRect.y = static_cast<int>(srcY);
		srcRect.w = srcRect.h = tsize;

		sc->position.x = static_cast<float>(x);
		sc->position.y = static_cast<float>(y);
		sc->width = sc->height = static_cast<int>(tsize);
		sc->scale = tscale;
	}
	TileComponent(Entity& e, TileComponent& tc) : EComponent(e),
		tileTexture(tc.tileTexture),
		srcRect(tc.srcRect),
		dstRect(tc.dstRect),
		sc{ &e.getComponent<SpatialComponent>() }

	{
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<TileComponent>(*this);
	}
	~TileComponent() {
		SDL_DestroyTexture(tileTexture);
	}
	Vector2D getCenter() const {
		return sc->getCenter();
	}

	void draw() override {
		Game::camera.relateToCamera(dstRect, sc->position, static_cast<int>(sc->width * sc->scale), static_cast<int>(sc->height * sc->scale));
		TextureManager::Draw(tileTexture, &srcRect, dstRect, SDL_FLIP_NONE, 0.0);
	}
};