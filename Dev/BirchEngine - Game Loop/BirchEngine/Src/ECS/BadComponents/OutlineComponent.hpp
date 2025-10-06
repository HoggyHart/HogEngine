#pragma once
#include "Src/ECS/ECS.hpp"
#include "Src/ECS/SpriteComponent.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
#include "Src/Game Management/TextureManager.hpp"

#include "Src/Utils/utils_SDL.hpp"

//FIX: this doesnt work. i think instead of outlining its basically a drawcomponent but restricted to a square around the entity bounds.
//		what i WANT to be done will be done with shaders
using namespace Pixels;
class OutlineComponent : public EComponent {
public:
	SpriteComponent* entitySprite{ nullptr };
	SDL_Rect dstRect{};
	OutlineComponent(Entity& e) : EComponent(e) {
		entitySprite = &e.getComponent<SpriteComponent>();
	};
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<OutlineComponent>(*this);
	}
	void init() override {
		this->getOwner()->reorderComponents({ this,&this->getOwner()->getComponent<SpriteComponent>() });
	}

	//not sure if this works. didnt try with a streaming texture
	void outlineTexture(SDL_Texture* tex) {
		Uint8 outlineThickness = 10;

		int w, h;
		Uint32 format;
		SDL_QueryTexture(tex, &format, nullptr, &w, &h);
		void* tmp;
		int pitch;
		SDL_LockTexture(tex, nullptr, &tmp, &pitch);
		std::cout << SDL_GetError();
		exit(10);
		Uint8* pixels = (Uint8*)tmp;

		for (std::size_t y = 0; y < h; ++y) {
			for (std::size_t x = 0; x < w; ++x) {
				//if it is transparent (i.e. isnt part of the green mask)
				if (pixels[y * pitch + x * 4 + 3] == 0) {
					//for each pixel surrounding it in outline thickness range
					checkSurroundingPixels(pixels + y * pitch + x * 4, x, y, w, h, pitch, outlineThickness);
				}
			}
		}
		SDL_UnlockTexture(tex);
	}

	void checkSurroundingPixels(Uint8* pixel, int x, int y, int w, int h, int pitch, int outlineThickness) {
		for (int dy = -outlineThickness; dy < outlineThickness; ++dy) {
			if (y + dy < 0 || y + dy >= h) continue;
			for (int dx = -outlineThickness; dx < outlineThickness; ++dx) {
				if (x + dx < 0 || x + dx >= w) continue;
				//if the surrounding pixel was green
				if (*(pixel + pitch * dy + dx * 4 + 3) == 255) {
					*(pixel + 0) = 255;
					*(pixel + 1) = 0;
					*(pixel + 2) = 0;
					*(pixel + 3) = 255;
					return;
				}
			}
		}
	}

	//afaik working surface outliner
	SDL_Texture* outlineExample() {
		std::string testImage = "assets/Images/Textures/complexImg.png";

		std::size_t outlineThickness = 10;
		SDL_Surface* img = IMG_Load(testImage.c_str());
		std::size_t w = img->w; std::size_t h = img->h;
		SDL_Surface* silhouette = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
		std::vector<unsigned char*> outline;
		for (std::size_t y = 0; y < h; ++y) {
			for (std::size_t x = 0; x < w; ++x) {
				if (getPixel(img, x, y).a == 255) {
					setPixel(silhouette, x, y, 0, 255, 0, 255);
				}
			}
		}

		dstRect.x = dstRect.y = 0;
		dstRect.w = dstRect.h = 750;

		SDL_Surface* outlineSurf = SDL_CreateRGBSurface(0, w, h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);

		//for each pixel
		for (std::size_t y = 0; y < h; ++y) {
			for (std::size_t x = 0; x < w; ++x) {
				//if it is transparent (i.e. isnt part of the green mask)
				if (getPixel(silhouette, x, y).a == 0) {
					//for each pixel surrounding it in outline thickness range
					temp(silhouette, x, y, outlineThickness, outlineSurf);
				}
			}
		}
		SDL_Texture* outlineTex = SDL_CreateTextureFromSurface(Game::renderer, outlineSurf);
		SDL_FreeSurface(img);
		SDL_FreeSurface(outlineSurf);
		return outlineTex;
	}
	//used for checking surrounding pixels of a surface
	void temp(SDL_Surface* s, int x, int y, int outlineThickness, SDL_Surface* newSurface) {
		for (int y2 = y - outlineThickness; y2 < y + outlineThickness; ++y2) {
			if (y2 < 0 || y2 >= s->h) continue;
			for (int x2 = x - outlineThickness; x2 < x + outlineThickness; ++x2) {
				if (x2 < 0 || x2 >= s->w) continue;
				//if the surrounding pixel was green
				if (getPixel(s, x2, y2).a == 255) {
					setPixel(newSurface, x, y, 255, 0, 0, 255);
					return;
				}
			}
		}
	}

	void draw() override {
		TextureManager::Draw(entitySprite->texture, &entitySprite->srcRect, dstRect, SDL_FLIP_NONE, 0);
	}
};