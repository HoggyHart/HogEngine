#pragma
#include "components.hpp"
#include "Src/Game Management/GameTime.hpp"
//replace with more advanced FadeIntoComponent at some point, whjich can fade from colour to colour too?
class FadeAwayComponent : public EComponent {
public:
	Uint32 fadeDuration;
	int remainingFadeDuration;
	Uint8 currentAlpha{ 255 };
	SDL_Texture* toFade = nullptr;
	FadeAwayComponent(Entity& e, Uint32 fadeDuration) : EComponent(e), fadeDuration(fadeDuration), remainingFadeDuration(fadeDuration) {
	}
	FadeAwayComponent(Entity& e, FadeAwayComponent& fac) : EComponent(e),
		fadeDuration(fac.fadeDuration),
		remainingFadeDuration(fac.remainingFadeDuration),
		currentAlpha(fac.currentAlpha) {
		//toFade gets sorted in init()
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<FadeAwayComponent>(*this);
	}
	void init() override {
		if (this->getOwner()->hasComponent<TextComponent>()) { toFade = this->getOwner()->getComponent<TextComponent>().texture; this->getOwner()->reorderComponents({ this,&this->getOwner()->getComponent<TextComponent>() }); }
		else if (this->getOwner()->hasComponent<SpriteComponent>()) { toFade = this->getOwner()->getComponent<SpriteComponent>().texture; this->getOwner()->reorderComponents({ this,&this->getOwner()->getComponent<SpriteComponent>() }); }
		else if (this->getOwner()->hasComponent<TileComponent>()) { toFade = this->getOwner()->getComponent<TileComponent>().tileTexture; this->getOwner()->reorderComponents({ this,&this->getOwner()->getComponent<TileComponent>() }); }
	}
	void draw() override {
		remainingFadeDuration -= static_cast<int>(GameTime::deltaTime);
		if (remainingFadeDuration < 0) currentAlpha = 0;
		else	currentAlpha = static_cast<Uint8>(255 * (static_cast<float>(remainingFadeDuration) / static_cast<float>(fadeDuration)));

		SDL_SetTextureAlphaMod(toFade, currentAlpha);
	}
};