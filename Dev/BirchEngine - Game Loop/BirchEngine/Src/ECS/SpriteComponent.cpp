#include "SpriteComponent.hpp"

void SpriteController::update()
{
	//controls animation and flip //not sure this is the right place, but it fits  enough until I have a dedicated "SpriteControl" section of code (e.g. if frozen by ice mid animation, wouldnt play "Move" animation while sliding)
	if (sc->getOwner()->hasComponent<MovementComponent>() && sc->getOwner()->getComponent<MovementComponent>().velocity.x != 0.0f) {
		sc->play("Move", playType_playOnce);
		if (sc->getOwner()->getComponent<MovementComponent>().velocity.x < 0) {
			sc->spriteFlip = SDL_FLIP_HORIZONTAL;
		}
		else sc->spriteFlip = SDL_FLIP_NONE;
	}
}