#pragma once
#include "SpatialComponent.hpp"
#include "MovementComponent.hpp"
#include "SDL.h"
#include "Src/Game Management/TextureManager.hpp"
#include "Src/Game Management/GameTime.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
#include "Src/Game Management/ObjectManagers/AssetManager.hpp"
#include <map>

enum playTypes { playType_playOnce, playType_loop, playType_playOnceAndFreeze };
class SpriteComponent;
struct SpriteController {
	SpriteComponent* sc;
	SpriteController(SpriteComponent* sc) : sc(sc) {}
	void update();
};

struct Animation {
public:
	int index = 0;
	int frames = 0;
	Uint32 frameDelay = 0;

	Animation() {};
	Animation(int i, int f, int fd) : index(i), frames(f), frameDelay(fd) {};
};

class SpriteComponent : public EComponent {
public:
	SpatialComponent* entityTransform{ nullptr };
	SpriteController controller{ this };


	SDL_Texture* texture{ nullptr };
	std::string textureID{ "" };

	SDL_Rect srcRect{}, dstRect{};

	std::string lastLoopingAnim = "";

	bool animated = false;
	int spriteFrameCount = 1;
	Uint32 spriteDelay = 150; //ms
	Uint32 lastFrame = 0;
	playTypes playType = playType_loop;

	int animIndex = 0;

	//char = animation name
	std::map<std::string, Animation> animations;

	SDL_RendererFlip spriteFlip = SDL_FLIP_NONE;
	SpriteComponent(Entity& e) : EComponent(e) { setDefaultRects(); };
	SpriteComponent(Entity& e, std::string filepath) : EComponent(e) {
		setTexture(filepath);
		setDefaultRects();
	}
	SpriteComponent(Entity& e, SpriteComponent& sc) : EComponent(e),
		entityTransform(&e.getComponent<SpatialComponent>()),
		texture(sc.texture),
		srcRect(sc.srcRect),
		dstRect(sc.dstRect),
		lastLoopingAnim(sc.lastLoopingAnim),
		animated(sc.animated),
		spriteFrameCount(sc.spriteFrameCount),
		spriteDelay(sc.spriteDelay),
		lastFrame(sc.lastFrame),
		playType(sc.playType),
		animIndex(sc.animIndex),
		animations(sc.animations),
		spriteFlip(sc.spriteFlip)
	{
	}
	SpriteComponent(Entity& e, std::string spriteID, bool isAnimated, std::map<std::string, Animation> spriteAnimations) : EComponent(e) { //animatedsprite constructor can be used if a sprite has multiple states
		//i.e. Animation(0,1,1) for srcRect at 0,0 with 1 frame (i.e. state 1 as red light) with delay of 1ms per frame (doesnt matter cos delay is only considered if animated is true)
		//i.e. Animation(1,1,1) for srcRect at 0,32 with 1 frame blah blah blah etc etc. (i.e. state 2, yellow light)
		setTexture(spriteID);

		animated = isAnimated;

		animations = spriteAnimations;
		setDefaultRects();
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<SpriteComponent>(*this);
	}
	~SpriteComponent() {
		//texture need not be SDL_DestroyTexture(texture);'d because the texture is managed by AssetManager
	}

	void setDefaultRects() {
		entityTransform = &this->getOwner()->getComponent<SpatialComponent>();

		srcRect.x = srcRect.y = 0;
		srcRect.w = entityTransform->width;
		srcRect.h = entityTransform->height;
		dstRect.w = static_cast<int>(entityTransform->width * entityTransform->scale);
		dstRect.h = static_cast<int>(entityTransform->height * entityTransform->scale);
	}
	void setTexture(std::string texID) {
		texture = Game::assetManager->getTexture(texID);
		textureID = texID;
	}

	void fixedUpdate(bool EOI) override {
		controller.update();
	}

	void draw() override {
		Game::camera.relateToCamera(dstRect, entityTransform->position, static_cast<int>(entityTransform->width * entityTransform->scale), static_cast<int>(entityTransform->height * entityTransform->scale));

		if (animated && GameTime::totalTime - lastFrame >= spriteDelay) { //if time for next frame
			srcRect.x += entityTransform->width; //go to next frame

			if (srcRect.x >= entityTransform->width * spriteFrameCount) { //if has gone past last frame
				if (this->playType == playType_playOnce) { if (animations[lastLoopingAnim].index == animIndex) srcRect.x = 0; play(lastLoopingAnim, playType_loop); }//AND animation does not loop, play last looping animation
				else if (this->playType == playType_playOnceAndFreeze) srcRect.x -= entityTransform->width;
				else srcRect.x = 0; //if looping, start at beginning of animation
			}
			lastFrame = GameTime::totalTime;
		}
		TextureManager::Draw(texture, &srcRect, dstRect, spriteFlip, entityTransform->rotation.asDegrees());
	}

	void draw(ViewPort* camera) override {
		if (camera == nullptr) {
			Game::camera.relateToDisplay(dstRect, entityTransform->position, static_cast<int>(entityTransform->width * entityTransform->scale), static_cast<int>(entityTransform->height * entityTransform->scale));
		}
		else {
			camera->relateToCamera(dstRect, entityTransform->position, entityTransform->width, entityTransform->height);
		}

		TextureManager::Draw(texture, &srcRect, dstRect, spriteFlip, entityTransform->rotation);
	}

	void play(std::string animName, playTypes playType) { //change to implement loop, play once, and play once and freeze
		switch (playType) {
		case playType_loop:
			lastLoopingAnim = animName; //if this animation loops, then it will be gone back to when the next non-looping animation is finished
			break;
		}
		this->playType = playType;

		if (animations[animName].index != animIndex) {//if new animation is NOT the same as the previous animation (i.e. not the same animation index):s start from frame 1
			animIndex = animations[animName].index;
			srcRect.x = 0;
		}
		srcRect.y = animIndex * entityTransform->height;

		spriteFrameCount = animations[animName].frames;
		spriteDelay = animations[animName].frameDelay;
	}
};