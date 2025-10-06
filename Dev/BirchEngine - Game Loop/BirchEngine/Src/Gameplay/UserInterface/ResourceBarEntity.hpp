#pragma once
#include "Src/ECS/components.hpp"
#include "Src/Game Management/Game.hpp"

struct Bar {
	float maxValue;
	float& curValue;
	SDL_Color barColor;
	SDL_Rect dstRect = SDL_Rect();
	SpatialComponent* entityTransform = nullptr;

	std::size_t barNumber = 0;
	Bar(float& maxVal, SDL_Color colour) : maxValue(maxVal), curValue(maxVal), barColor(colour) {
	}

	void init(SpatialComponent*& tf, std::size_t barNumber) {
		entityTransform = tf;
		this->barNumber = barNumber;
		dstRect.x = static_cast<int>((entityTransform->position.x - entityTransform->width * entityTransform->scale * 0.25) * Game::camera.zoomAmount - Game::camera.x);
		dstRect.y = static_cast<int>((entityTransform->position.y + entityTransform->height * entityTransform->scale) * Game::camera.zoomAmount - Game::camera.y + barNumber * 11);
		dstRect.h = 10;
		dstRect.w = static_cast<int>(static_cast<float>(entityTransform->width) * 1.5);
	}
	void update() {
		dstRect.x = static_cast<int>((entityTransform->position.x - entityTransform->width * entityTransform->scale * 0.25) * Game::camera.zoomAmount - Game::camera.x);
		dstRect.y = static_cast<int>((entityTransform->position.y + entityTransform->height * entityTransform->scale) * Game::camera.zoomAmount - Game::camera.y + barNumber * 11);

		dstRect.w = static_cast<int>(static_cast<float>(entityTransform->width) * entityTransform->scale * Game::camera.zoomAmount * 1.5f * curValue / maxValue);
		if (dstRect.w < 0) dstRect.w = 0;
	}
	void draw() const {
		SDL_SetRenderDrawColor(Game::renderer, barColor.r, barColor.g, barColor.b, barColor.a);
		SDL_RenderFillRect(Game::renderer, &dstRect);
	}
};

class ResourcesComponent : public EComponent {
private:
	std::vector<Bar> bars;
	SpatialComponent* entityTransform = nullptr;

public:

	ResourcesComponent(Entity& e, float& maxVal, SDL_Color colour) : EComponent(e) {
		bars.push_back(Bar(maxVal, colour));
	}
	void init() override {
		entityTransform = &this->getOwner()->getComponent<SpatialComponent>();
		bars[0].init(entityTransform, 0);
	}
	void fixedUpdate(bool EOI) override {
		for (Bar& bar : bars) bar.update();
	}
	void draw() override {
		for (Bar& bar : bars) bar.draw();
	}
	void interUpdate() override {
		for (Bar& bar : bars) bar.update();
	}
	void addResource(float& maxVal, SDL_Color color) {
		bars.push_back(Bar(maxVal, color));
		bars.back().init(entityTransform, bars.size() - 1);
	}
};