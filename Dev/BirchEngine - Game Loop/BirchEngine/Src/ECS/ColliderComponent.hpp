#pragma once
#include <string>
#include "SDL.h"
#include "components.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Game Management/ObjectManagement.hpp"

enum HitboxShape{ Rectangle, Circle, FancyRect };

struct CollisionAreaBase {
	HitboxShape shape;
	SpatialComponent* entitySpace;
	CollisionAreaBase() = delete;
	CollisionAreaBase(SpatialComponent& sc) : entitySpace(&sc) {}
	virtual void reallign() {};
};

struct CollisionAreaRect : public CollisionAreaBase{
	SDL_Rect hitbox;

	CollisionAreaRect(SpatialComponent& sc) : CollisionAreaBase(sc) { shape = Rectangle; reallign(); }
	void reallign() override {
		hitbox = entitySpace->toRect();
	}
};

struct WHE_Circle {
	Vector2D centre;
	float radius;
};
struct CollisionAreaCirc : public CollisionAreaBase{
	
	WHE_Circle area;

	CollisionAreaCirc(SpatialComponent& sc) : CollisionAreaBase(sc) { shape = Circle; reallign(); }

	void reallign() override {
		area.centre = entitySpace->getCenter();
		area.radius = entitySpace->width * entitySpace->height;
	}
};

struct WHE_FRect : public SDL_Rect {
	Angle rotation;
};
struct CollisionAreaFancyRect : public CollisionAreaBase{

	WHE_FRect area;
	CollisionAreaFancyRect(SpatialComponent& sc) : CollisionAreaBase(sc) { shape = FancyRect; reallign(); }
	
	void reallign() override {
		area.x = entitySpace->position.x;
		area.y = entitySpace->position.y;
		area.w = entitySpace->width * entitySpace->scale;
		area.h = entitySpace->height * entitySpace->scale;
		area.rotation = entitySpace->rotation;
	}
};

union CollisionArea {
	HitboxShape shape;

	CollisionAreaRect rect;
	CollisionAreaCirc circ;
	CollisionAreaFancyRect fRect;
};

enum ColliderType{ Creature, Terrain, Structure, Projectile};
enum ColliderInteraction{DEFAULT_NO, DEFAULT_YES, FORCED_NO, FORCED_YES}; //higher value enum means higher priority. DEFAULT is most common and can be overriden if entity 2 has FORCED behaviour. 
// i.e. using abbreviations of DN and DY, FN and FY, and C for creature and W for WALL:
// Normal C normal W: C=DY, W=DY: both yes so collide
// Ghost C normal W: C=DN, W=DY: this to me is a bit unclear, maybe this should result in a collision anyway and they need specific if(C=ghost)/if(W=normal wall) type things
//	BUT i think better is either C=FN to say "I do NOT collide with walls" or W=FY to say "I ALWAYS collide with creatures"
//  I think the	C=FN would be the right choice here, because then it allows for a "super wall" or map border to be W=FY to say "I NEED to interact with things, you can't avoid me"
// ------I'm also thinking about special interactions, where by default a wall pushes you away but you have the power to melt the wall to make an opening or smth
// ------This would be an "I can collide with you, but you cant collide with me" type thing. I need to think of a proper way to handle this.
// 
//this is so when checking if two should collide you can comapre with a simple "<" comparison:
// i.e. a.thisType = Projectile, and b.thisType = Structure, where a pierces walls but b is first normal and 2nd blocks certain types of projectiles (i.e. arrows/bombs)
//	if (a.collideBehaviour[Terrain] > b.collideBehaviour 
// 

/*	
Indicates type of method to call regarding other entity in collision
	None: no method to call on other entity
	Standard: has method to call, also receives other entity's method
	Special: calls a method that may or may not ignore other entity's collision (indicated by bool return value from typedef CollisionMethod )
_-----------
	In terms of priority it goes Special > Standard > None
	Standard means the method gets run and thats that, so fancy stuff
	Special means this method may ignore the other method conditionally and as such should come first
*/
enum MethodType{None, Standard, Special};


typedef std::string CollisionCallbackID;

struct CollisionInteractionInfo {
	bool ignored{ false };
	//method type indicates method contents/priority
	MethodType type{ None };

	CollisionCallbackID methodID{ 0u };
	CollisionInteractionInfo() {}
	CollisionInteractionInfo(const CollisionInteractionInfo& original) : type( original.type ), methodID( original.methodID ) {}
};
typedef std::map<const ColliderType, CollisionInteractionInfo> CollisionResponseMap;
class ColliderComponent : public EComponent {
	
	CollisionArea hitbox{}; //contains info about hitbox shape, size, pos
	ColliderType thisType; //contains this hitbox's entity type 

	bool ignoreCollisions{ false };
	CollisionResponseMap collisionResponses; 

public:
	ColliderComponent(Entity& e, ColliderType type, HitboxShape shape, CollisionCallbackID onCollision) : EComponent(e), thisType{ type } {
		hitbox.shape = shape;
	
		switch (hitbox.shape) {
			case Rectangle:
				new (&hitbox.rect) CollisionAreaRect{this->getOwner()->getComponent<SpatialComponent>()};
				break;
			case Circle:
				new (&hitbox.circ) CollisionAreaCirc{ this->getOwner()->getComponent<SpatialComponent>() };
				break;
			case FancyRect:
				new (&hitbox.fRect) CollisionAreaFancyRect{ this->getOwner()->getComponent<SpatialComponent>() };
				break;
		}
	}

	CollisionArea&					getHitbox() { return hitbox; }
	
	inline void						setCollisionResponse(ColliderType colliderType, MethodType type, CollisionCallbackID method) {
		collisionResponses[colliderType].type = type;
		collisionResponses[colliderType].methodID=method;
	}
	inline CollisionInteractionInfo getCollisionResponseInfo(const ColliderType colliderType) const { return collisionResponses.at(colliderType);}
	inline bool						hasMethodFor(const ColliderType t) const{ return collisionResponses.at(t).type != MethodType::None; }
	inline void						setIgnoreCollisions(bool ignore) { ignoreCollisions = ignore; }
	inline bool						ignoresCollisions() const { return ignoreCollisions; }
	inline bool						ignoresType(const ColliderType t) const { return collisionResponses.at(t).ignored; }
	inline ColliderType				getType() const { return thisType; }
	
	void reallignHB() {
		switch (hitbox.shape) {
		case Rectangle:
			hitbox.rect.reallign();
			break;
		case Circle:
			hitbox.circ.reallign();
			break;
		case FancyRect:
			hitbox.fRect.reallign();
			break;
		}
	}
	void fixedUpdate(bool EOI) override {
		reallignHB();
	}
	void interUpdate() override {
		reallignHB();
	}
};