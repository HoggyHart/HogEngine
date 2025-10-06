#pragma once
#include "Src/ECS/ECS.hpp"
#include "SpatialComponent.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Game Management/Game.hpp"
//tmeporary dogshit
typedef std::pair<float, float> Point;
typedef std::vector<Point> PointsList;

//FIX: MASSIVE OVERHAUL TO BE DONE HERE:
// NEED:
//	1. To be able to FILL a shape
//	2. To be able to have multiple shapes
// WOULD BE NICE:
//	1. To have some sort of shape control module. I.e. if drawcomponent draws eyes, allow it to move the pupil within the eye

class DrawComponent : public EComponent {
public:
	SDL_Color clr;
	bool fill{ false };
	Uint32 lineW{ 1 };
	//for drawing statModFlat to the window
	PointsList points;

	//for drawing relative to a SpatialComponent. no need to actually use a whole spatial component, just the position of it.
	Vector2D* relativePoint = nullptr;
	PointsList offsets;

	DrawComponent(Entity& e, PointsList points, SDL_Color color) : EComponent(e), points(points), clr(color) { fixedUpdate(false); }
	DrawComponent(Entity& e, Vector2D* relativePoint, PointsList points, SDL_Color color) : EComponent(e), relativePoint(relativePoint), points(points.size(), { 0.0f,0.0f }), offsets(points), clr(color) { fixedUpdate(false); }
	DrawComponent(Entity& e, SpatialComponent& sc, SDL_Color color) : EComponent(e), relativePoint(&sc.position),
		points(5, { 0.0f,0.0f }),
		offsets({ {0.0f,0.0f},{sc.width * sc.scale,0.0f},{sc.width * sc.scale,sc.height * sc.scale},{0.0f,sc.height * sc.scale},{0.0f,0.0f} }), clr(color) {
		fixedUpdate(false);
	}
	DrawComponent(Entity& e, DrawComponent& dc) : EComponent(e),
		clr(dc.clr),
		fill(dc.fill),
		lineW(dc.lineW),
		points(dc.points),
		offsets(dc.offsets) {
		if (dc.relativePoint != nullptr
			&& dc.getOwner()->hasComponent<SpatialComponent>()
			&& &dc.getOwner()->getComponent<SpatialComponent>().position == dc.relativePoint) {
			relativePoint = &e.getComponent<SpatialComponent>().position;
		}
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<DrawComponent>(*this);
	}
	void fixedUpdate(bool EOI) override {
		if (relativePoint) {
			for (std::size_t i = 0; i < points.size(); ++i) {
				points[i] = { relativePoint->x + offsets[i].first, relativePoint->y + offsets[i].second };
			}
		}
	}
	void draw() override {
		SDL_SetRenderDrawColor(Game::renderer, clr.r, clr.g, clr.b, clr.a);
		PointsList::iterator it = points.begin();
		Point* a = &*it;
		while (++it != points.end()) {
			Point* b = &*it;
			SDL_RenderDrawLine(Game::renderer,
				static_cast<int>(a->first * Game::camera.zoomAmount) - Game::camera.x,
				static_cast<int>(a->second * Game::camera.zoomAmount) - Game::camera.y,
				static_cast<int>(b->first * Game::camera.zoomAmount) - Game::camera.x,
				static_cast<int>(b->second * Game::camera.zoomAmount) - Game::camera.y);
			a = &*it;
		}
	}

	void draw(ViewPort* cam) override {
		SDL_SetRenderDrawColor(Game::renderer, clr.r, clr.g, clr.b, clr.a);
		PointsList::iterator it = points.begin();
		Point* a = &*it;
		while (++it != points.end()) {
			Point* b = &*it;
			SDL_RenderDrawLine(Game::renderer, static_cast<int>(a->first), static_cast<int>(a->second), static_cast<int>(b->first), static_cast<int>(b->second));
			a = &*it;
		}
	}
};