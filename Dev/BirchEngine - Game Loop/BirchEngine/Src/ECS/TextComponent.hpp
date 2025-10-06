#pragma once
#include "components.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
#include "SDL_ttf.h"
enum Allignment {
	NONE,
	TOP_LEFT, TOP_CENTER, TOP_RIGHT,
	CENTER_LEFT, CENTER, CENTER_RIGHT,
	BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT
};

class TextComponent : public EComponent {
public:
	SDL_Texture* texture = nullptr;
	SDL_Color* fontColour = nullptr;
	std::string font = "Consolasx24";
	std::string text = "Text";
	SDL_Rect textArea = SDL_Rect();
	//pointer so can have nullptr i.e. no in-game position
	SpatialComponent* allignmentArea;
	Allignment allignment;
	std::pair<int, int> allignmentOffset{ 0,0 };
	bool staticPos{ true };

	inline void updateTexture(std::string text) {
		SDL_Surface* tmp = TTF_RenderText_Blended(Game::assetManager->getFont(font), text.c_str(), *fontColour);

		// now you can convert it into a texture
		texture = SDL_CreateTextureFromSurface(Game::renderer, tmp);
		SDL_FreeSurface(tmp);

		SDL_QueryTexture(texture, nullptr, nullptr, &textArea.w, &textArea.h);
	}
public:
	//if entity has SpatialComponent, text should be non-statically positioned
	//if entity does not have SpatialComponent, one should be created and the text should be static.

	//if given x and y, those are treated as offsets from the allignment position,
	//generally used for a FreeLayout position
	TextComponent(Entity& e, Allignment all, int x, int y, std::string text, std::string font, SDL_Color& colour) : EComponent(e), allignment(all), allignmentOffset({ x,y }) {
		if (e.hasComponent<SpatialComponent>()) {
			allignmentArea = &e.getComponent<SpatialComponent>();
		}
		else {
			allignmentArea = &e.addComponent<SpatialComponent>(0.0f, 0.0f, Game::camera.displaySize.w, Game::camera.displaySize.h, 1.0f);
		}

		this->text = text;
		this->font = font;
		this->fontColour = &colour;
		updateTexture(text);
		fixedUpdate(false);
	}
	TextComponent(Entity& e, Allignment all, std::string text, std::string font, SDL_Color& colour) : EComponent(e), allignment(all), allignmentOffset({ 0,0 }) {
		if (e.hasComponent<SpatialComponent>()) {
			allignmentArea = &e.getComponent<SpatialComponent>();
		}
		else {
			allignmentArea = &e.addComponent<SpatialComponent>(0.0f, 0.0f, Game::camera.displaySize.w, Game::camera.displaySize.h, 1.0f);
		}

		this->text = text;
		this->font = font;
		this->fontColour = &colour;
		updateTexture(text);
		fixedUpdate(false);
	}
	TextComponent(Entity& e, TextComponent& tc) : EComponent(e),
		texture{ tc.texture },
		fontColour(tc.fontColour),
		font(tc.font),
		text(tc.text),
		textArea(tc.textArea),
		allignment(tc.allignment),
		allignmentOffset(tc.allignmentOffset),
		staticPos(tc.staticPos)
	{
		if (e.hasComponent<SpatialComponent>()) {
			allignmentArea = &e.getComponent<SpatialComponent>();
		}
		else {
			allignmentArea = &e.addComponent<SpatialComponent>(0.0f, 0.0f, Game::camera.displaySize.w, Game::camera.displaySize.h, 1.0f);
		}
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<TextComponent>(*this);
	}

	~TextComponent() {
		SDL_DestroyTexture(texture);
	}
	void init() override {}
	// as TTF_RenderText_Solid could only be used on
	// SDL_Surface then you have to create the surface first
	void changeText(std::string text) {
		SDL_DestroyTexture(texture); //crash ONLY occurs with slashAtCursor as action and when not deleting expired links
		this->text = text;
		updateTexture(text);
	}
	void changeColour(SDL_Color& colour) {
		SDL_DestroyTexture(texture);
		this->fontColour = &colour;
		updateTexture(text);
	}

	void fixedUpdate(bool EOI) override {
		allignTextbox(allignment);
		if (this->getOwner()->isGameEntity()) {
			textArea.x = static_cast<int>(textArea.x * Game::camera.zoomAmount) - Game::camera.x;
			textArea.y = static_cast<int>(textArea.y * Game::camera.zoomAmount) - Game::camera.y;
		}
	}

	void allignTextbox(Allignment a) {
		allignment = a;
		int left{ static_cast<int>(allignmentArea->position.x) };
		int xCenter{ static_cast<int>(allignmentArea->position.x + allignmentArea->width / 2.0f - textArea.w / 2.0f) };
		int right{ static_cast<int>(allignmentArea->position.x + allignmentArea->width - textArea.w) };

		int top{ static_cast<int>(allignmentArea->position.y) };
		int yCenter{ static_cast<int>(allignmentArea->position.y + allignmentArea->height / 2.0f - textArea.h / 2.0f) };
		int bottom{ static_cast<int>(allignmentArea->position.y + allignmentArea->height - textArea.h) };
		switch (allignment) {
		case TOP_LEFT:
			textArea.x = left;
			textArea.y = top;
			break;
		case TOP_CENTER:
			textArea.x = xCenter;
			textArea.y = top;
			break;
		case TOP_RIGHT:
			textArea.x = right;
			textArea.y = top;;
			break;
		case CENTER_LEFT:
			textArea.x = left;
			textArea.y = yCenter;
			break;
		case CENTER:
			textArea.x = xCenter;
			textArea.y = yCenter;
			break;
		case CENTER_RIGHT:
			textArea.x = right;
			textArea.y = yCenter;
			break;
		case BOTTOM_LEFT:
			textArea.x = left;
			textArea.y = bottom;
			break;
		case BOTTOM_CENTER:
			textArea.x = xCenter;
			textArea.y = bottom;
			break;
		case BOTTOM_RIGHT:
			textArea.x = right;
			textArea.y = bottom;
			break;
		case NONE:
			textArea.x = 0;
			textArea.y = 0;
			break;
		default:
			break;
		}
		textArea.x += allignmentOffset.first;
		textArea.y += allignmentOffset.second;
	}

	void draw() override {
		SDL_RenderCopy(Game::renderer, texture, nullptr, &textArea);
	}
	void draw(ViewPort* camera) override {//POTFIX: WHAT? something something idk <-- no clue wtf talking about. migth need a fix here? idk
		if (!camera) {
			SDL_RenderCopy(Game::renderer, texture, nullptr, &textArea);
		}
	}
	SDL_Rect asRect() const {
		return textArea;
	}
};