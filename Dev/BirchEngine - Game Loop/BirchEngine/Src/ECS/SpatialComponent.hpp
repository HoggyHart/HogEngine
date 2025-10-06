#pragma once
#include "Src/Game Management/Game.hpp"
#include "Src/ECS/ECS.hpp"
#include "Src/Utils/Vector2D.hpp"
#include "Src/Gameplay/CreatureStuff/StatsAndModifiers.hpp"
class LinkComponent;

class SpatialComponent : public EComponent {
public:
	Vector2D position;
	int height = 32;
	int width = 32;
	float scale = 1;
	Angle rotation{ 0 }; //refactor to use radians

	std::vector<LinkComponent*> links;
	SpatialComponent(Entity& e) : EComponent(e) {
		position.zero();
	}
	SpatialComponent(Entity& e, float sc) : EComponent(e) {
		scale = sc;
	}
	SpatialComponent(Entity& e, float x, float y, int w, int h, float sc) : EComponent(e) {
		position.x = x;
		position.y = y;
		height = h;
		width = w;
		scale = sc;
	}
	SpatialComponent(Entity& e, SpatialComponent& sc) : EComponent(e),
		position(sc.position),
		height(sc.height),
		width(sc.width),
		scale(sc.scale),
		rotation(sc.rotation)
	{
	}

	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<SpatialComponent>(*this);
	}
	~SpatialComponent() {
		closeAllLinks();
	}
	void init() override {}
	void fixedUpdate(bool EOI) override {
		/*for (LinkComponent*& l : linksOut) {
			l->enforce();
		}*/
	}

	Vector2D getCenter() const {
		return Vector2D(position.x + (scale * width / 2.0f), position.y + (scale * height / 2.0f));
	}

	SDL_Rect toRect() const {
		return SDL_Rect{ static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(width * scale), static_cast<int>(height * scale) };
	}
	SDL_Rect& toRect(SDL_Rect& r) const {
		r.x = static_cast<int>(position.x);
		r.y = static_cast<int>(position.y);
		r.w = static_cast<int>(width * scale);
		r.h = static_cast<int>(height * scale);
		return r;
	}

	//template<typename LinkType, typename ...LArgs>
	//LinkType* addLink(SpatialComponent* tc, LArgs ...args) {
	//	if (tc) { //if non-nullptr tc provided
	//		LinkType* link = new LinkType(this, tc, args...);
	//		this->linksOut.push_back(link);
	//		tc->linksIn.push_back(link);
	//		return link;
	//	}
	//	return nullptr;
	//}
	LinkComponent& addLink(SpatialComponent& tc, std::pair<Vector2D, Vector2D> offsets);
	LinkComponent& addLink(SpatialComponent& tc);

	void closeLink(LinkComponent* link);
	void closeAllLinks();
};

inline std::ostream& operator<<(std::ostream& os, const SpatialComponent& sc) {
	return os << "{Pos: " << sc.position.x << ',' << sc.position.y << " | Size: " << sc.width << "," << sc.height << " *" << sc.scale << "}";
}