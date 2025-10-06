#include "TextureManager.hpp"
#include "Game.hpp"
#include "Src/ECS/TileComponent.hpp"

SDL_Texture* TextureManager::LoadTexture(const char* filename) {
	SDL_Surface* tmpSurface = IMG_Load(filename);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(Game::renderer, tmpSurface);
	SDL_FreeSurface(tmpSurface);
	return texture;
}

void TextureManager::Draw(SDL_Texture* texture, SDL_Rect* src, SDL_Rect& dst, SDL_RendererFlip flip, double rotation) {
	SDL_RenderCopyEx(Game::renderer, texture, src, &dst, rotation, nullptr, flip);
	TextureManager::resetTextureRenderMods(texture);
}

void TextureManager::DrawMap(std::vector<Entity*>& group)
{
	if (group.size() == 0) return;
	SDL_Texture* tex = group[0]->getComponent<TileComponent>().tileTexture;
	for (Entity*& e : group) {
		TileComponent& tc = e->getComponent<TileComponent>();
		tc.dstRect.x = static_cast<int>(tc.sc->position.x * Game::camera.zoomAmount - Game::camera.x);
		tc.dstRect.y = static_cast<int>(tc.sc->position.y * Game::camera.zoomAmount - Game::camera.y);
		tc.dstRect.w = tc.dstRect.h = static_cast<int>(tc.sc->width * tc.sc->scale * Game::camera.zoomAmount);
		Game::camera.relateToCamera(tc.dstRect, tc.sc->position, static_cast<int>(tc.sc->width * tc.sc->scale), static_cast<int>(tc.sc->height * tc.sc->scale));
		SDL_RenderCopyEx(Game::renderer, tex, &tc.srcRect, &tc.dstRect, 0, nullptr, SDL_FLIP_NONE);
	}
	TextureManager::resetTextureRenderMods(tex);
}