#pragma once
#include "SDL.h"
#include "Vector2D.hpp"
struct ViewPort : public SDL_Rect {
public:
	float zoomAmount{ 1.0f };
	SDL_Rect displaySize{};
	SDL_Rect worldDisplay{};
	ViewPort() {};
	ViewPort(int x, int y, int w, int h) : displaySize{ x,y,w,h }, worldDisplay{ x,y,w,h }, SDL_Rect { x, y, w, h } {
	};

	SDL_Rect getCameraDisplay() const { return displaySize; }

	inline void zoom(float mag, std::pair<float,float> limits) {
		zoomAmount += mag;
		if (zoomAmount < limits.first) zoomAmount = limits.first;
		else if (zoomAmount > limits.second) zoomAmount = limits.second;
		this->w = displaySize.w / zoomAmount;
		this->h = displaySize.h / zoomAmount;
	}

	inline void centerOn(Vector2D v) {
		this->x = static_cast<int>(v.x - this->w / 2.0f);
		this->y = static_cast<int>(v.y - this->h / 2.0f);
	}
	inline void bind(SDL_Rect bounds) {
		//horizontal binding
		if (bounds.x != -1) {
			if (this->x < bounds.x) this->x = bounds.x; //if too left
			else if (this->x > static_cast<int>((bounds.w - this->w) * zoomAmount)) this->x = static_cast<int>((bounds.w - this->w) * zoomAmount); //if too right
		}
		//vertical binding
		if (bounds.y != -1) {
			if (this->y < bounds.y) this->y = bounds.y;
			else if (this->y > static_cast<int>((bounds.h - this->h) * zoomAmount)) this->y = static_cast<int>((bounds.w - this->h) * zoomAmount);
		}
	}

	inline SDL_Rect& relateToCamera(SDL_Rect& output, SDL_Rect posAndSize) {
		output.x = static_cast<int>(posAndSize.x - this->x)*this->zoomAmount;
		output.y = static_cast<int>(posAndSize.y - this->y)*this->zoomAmount;
		output.w = static_cast<int>(posAndSize.w * this->zoomAmount);
		output.h = static_cast<int>(posAndSize.h * this->zoomAmount);
		return output;
	}
	inline void relateToDisplay(SDL_Rect& relativeRect, Vector2D& pos, int w, int h) {
		relativeRect.x = static_cast<int>(pos.x);
		relativeRect.y = static_cast<int>(pos.y);
		relativeRect.w = w;
		relativeRect.h = h;
	}

	inline void relateToCamera(SDL_Rect& relativeRect, Vector2D& pos, int w, int h){
		
		float newPosPart = (pos.x - this->x) * this->zoomAmount;
		relativeRect.x = std::round(newPosPart);
		float newSizePart = w * this->zoomAmount;
		relativeRect.w = std::round(newSizePart);
		//rounding to avoid gaps between tiles
		if (std::modf(newPosPart + newSizePart, &newPosPart) >= 0.5f) {
			relativeRect.w += 1;
		}

		newPosPart = (pos.y - this->y) * this->zoomAmount;
		relativeRect.y = std::round(newPosPart);
		newSizePart = h * this->zoomAmount;
		relativeRect.h = std::round(newSizePart);
		//rounding to avoid gaps between tiles
		if (std::modf(newPosPart + newSizePart,&newPosPart) >= 0.5f) {
			 relativeRect.h += 1;
		}
	
	}

	inline bool isOnCamera(SDL_Rect& space) {
		return (space.x + space.w > this->x
			&& space.x <( this->x + this->w )
			&& space.y + space.h > this->y
			&& space.y < (this->y + this->h));
	}
};