#pragma once
#include "Src/ECS/ECS.hpp"
#include "SpatialComponent.hpp"
#include "MovementComponent.hpp"
#include "Src/Game Management/TextureManager.hpp"
#include "Src/Game Management/GameTime.hpp"
enum LinkDirection { LD_L1Oriented, LD_TwoWay, LD_L2Oriented };

class LinkComponent : public EComponent {
	std::string linkTag{ "" }; //tag i.e. if a characters action gives them the ability to sever other links, it should be able to search for the link by name maybe? idk. maybe tag isnt necessary

	bool active{ true };

	LinkDirection direction{ LD_L1Oriented };

	float minDistance{ 0.0f };
	float maxDistance{ -1.0f };
	float linkDistanceChange{ 0.0f }; //change per second

	double minAngle{ 0 };
	double maxAngle{ -1 };
	double linkRotationChange{ 0 }; //change per second
	bool rotateLinkEndSC{ false };

	SpatialComponent* lp1{ nullptr }; //link point 1 i.e. origin of the link

	SpatialComponent* lp2{ nullptr };

public:
	Vector2D lp1Offset{};
	Vector2D lp2Offset{};

	LinkComponent(Entity& e, SpatialComponent* l1, SpatialComponent* l2) : EComponent(e) {
		lp1 = l1;
		l1->getOwner()->addDependant(this->getOwner());
		lp2 = l2;
		l2->getOwner()->addDependant(this->getOwner());
		this->getOwner()->addGroup(EntityGroup::groupDrawingOverlay);
		this->getOwner()->addGroup(EntityGroup::groupInterpolatables);
	}
	LinkComponent(Entity& e, LinkComponent& lc) : EComponent(e),
		linkTag(lc.linkTag),
		active(lc.active),
		direction(lc.direction),

		minDistance(lc.minDistance),
		maxDistance(lc.maxDistance),
		linkDistanceChange(lc.linkDistanceChange),

		minAngle(lc.minAngle),
		maxAngle(lc.maxAngle),
		linkRotationChange(lc.linkRotationChange),
		rotateLinkEndSC(lc.rotateLinkEndSC),

		lp1(lc.lp1),
		lp2(lc.lp2) {
	}
	~LinkComponent() {
		unbindLinkPoints();
	}
	Component<Entity>* copyTo(Entity& e) override {
		//FIX: NOT SO SURE ABOUT THIS ONE
		//		JUST BASED ON THE concept of a LinkComponent(LinkComponent&) constructor, a link copy would probably have to be modified after creation to connect to different object/s
		return &e.addComponent<LinkComponent>(*this);
	}

	
	SpatialComponent* unbindLP1() {
		if (!lp1) return nullptr;
		SpatialComponent* l = lp1;
		lp1 = nullptr;
		l->links.erase(std::remove_if(l->links.begin(), l->links.end(), [this](LinkComponent*& b) {return this == b; }), l->links.end());
		return l;
	}
	SpatialComponent* unbindLP2() {
		if (!lp2) return nullptr;
		SpatialComponent* l = lp2;
		lp2 = nullptr;
		l->links.erase(std::remove_if(l->links.begin(), l->links.end(), [this](LinkComponent*& b) {return this == b; }), l->links.end());
		return l;
	}
	std::pair<SpatialComponent*, SpatialComponent*> unbindLinkPoints() {
		std::pair<SpatialComponent*, SpatialComponent*> scs{ nullptr,nullptr };
		scs.first = unbindLP1();
		scs.second = unbindLP2();
		//FIX: i think this causes the crashing problem with slashEntities
		// slash entity gets destroyed but the cascade destruction of link entities/trail entities doesnt fully expand as much as possible
		// so the deactivated ones get remvoed from group, then in the main entity removal bit more get deactivated by ~SpatialComponent.
		// this then means they get destroyed during the iteration of main entity removal but werent removed from the group
		// honestly not sure how this error didnt occur earlier
		this->getOwner()->destroy();
		return scs;
	}

	void setOffsets(Vector2D l1Offset, Vector2D l2Offset) {
		lp1Offset = l1Offset;
		lp2Offset = l2Offset;
	}

	void init() override {
	}

	void enforceDistance() {
		Vector2D p1 = lp1->position + lp1Offset;
		Vector2D p2 = lp2->position + lp2Offset;
		Vector2D p1top2 = p2 - p1;

		//check if beyond linkLimits
		bool limitExceeded{ false };
		if (p1top2.magnitude() > maxDistance) {
			p1top2 = p1top2.normalise() * maxDistance;
			limitExceeded = true;
		}
		else if (p1top2.magnitude() < minDistance) {
			p1top2 = p1top2.normalise() * minDistance;
			limitExceeded = true;
		}
		if (limitExceeded == false) return;

		//limit distance
		if (direction == LD_L1Oriented) {
			p2 = p1 + p1top2;
			lp2->position = p2 - lp2Offset;
		}
		else if (direction == LD_L2Oriented) {
			p1 = p2 - p1top2;
			lp1->position = p1 - lp1Offset;
		}
		else if (direction == LD_TwoWay) {
			Vector2D midway = p1 + (p2 - p1) / 2.0f;
			p2 = midway + p1top2 / 2.0f;
			p1 = midway - p1top2 / 2.0f;
			lp2->position = p2 - lp2Offset;
			lp1->position = p1 - lp1Offset;
		}
	}

	//this is ASS. FIX IT
	void enforceAngle() {
		Vector2D p1 = lp1->position + lp1Offset;
		Vector2D p2 = lp2->position + lp2Offset;
		Vector2D p1top2 = p2 - p1;
		Angle p1p2Angle = p1top2.toAngle();
		if (direction == LD_L1Oriented && p1p2Angle.between(minAngle, maxAngle)) return;
		if (direction == LD_L2Oriented && p1p2Angle.add(M_PI).between(minAngle, maxAngle)) return;
		double oppositeMidpoint = minAngle + (maxAngle - minAngle) / 2 + M_PI;
		//NOT SURE HOW TO LIMIT ANGLE PROPERLY

		//PLACEHOLDER
		if (direction == LD_L1Oriented) {
			p2 = p1 + Vector2D(Angle{ minAngle }) * p1top2.magnitude();
			lp2->position = p2 - lp2Offset;
		}
		else if (direction == LD_L2Oriented) {
			p1 = p2 + Vector2D(Angle{ minAngle }) * p1top2.magnitude();
			lp1->position = p1 - lp1Offset;
		}
	}

	void fixedUpdate(bool EOI) override {
		//if two things are linked
		if (lp1 && lp2) {
			//if distance must be limited
			if (maxDistance >= minDistance) {
				interpolateDistanceChange(false);
				enforceDistance();
			}

			//do angle stuff too
			if (maxAngle >= minAngle) {
				interpolateAngleChange(false);
				enforceAngle();
			}
		}
		else if (!lp1 && !lp2) {
			this->getOwner()->destroy();
		}
	}
	void interpolateDistanceChange(bool enforce) {
		if (linkDistanceChange != 0) {
			minDistance += linkDistanceChange * GameTime::deltaTime / 1000.0f;
			maxDistance += linkDistanceChange * GameTime::deltaTime / 1000.0f;
		}
		if (enforce) enforceDistance();
	}
	void interpolateAngleChange(bool enforce) {
		if (linkRotationChange != 0) {
			minAngle += (linkRotationChange * GameTime::deltaTime / 1000.0f);
			maxAngle += (linkRotationChange * GameTime::deltaTime / 1000.0f);
			enforceAngle();
			if (rotateLinkEndSC) {
				if (direction == LD_L1Oriented && lp2) lp2->rotation.add(linkRotationChange * GameTime::deltaTime / 1000.0f);
				if (direction == LD_L2Oriented && lp1) lp1->rotation.add(linkRotationChange * GameTime::deltaTime / 1000.0f);
			}
			if (enforce) enforceAngle();
		}
	}
	void interUpdate() override {
		if (!lp1 && !lp2) {
			this->getOwner()->destroy();
			return;
		}
		if (maxDistance >= minDistance) {
			interpolateDistanceChange(true);
		}
		if (maxAngle >= minAngle) {
			interpolateAngleChange(true);
		}
	}
	void draw() {
		if (Game::displayHitboxes && lp1 && lp2) {
			TextureManager::DrawLine(Game::renderer, static_cast<int>(lp1->position.x + lp1Offset.x), static_cast<int>(lp1->position.y + lp1Offset.y), static_cast<int>(lp2->position.x + lp2Offset.x), static_cast<int>(lp2->position.y + lp2Offset.y), SDL_Color{ 0,0,255,255 });
		}
	}

	void setMinDistance(float md) {
		minDistance = md;
	}
	void setMaxDistance(float md) {
		maxDistance = md;
	}
	void setLinkDistance(float dist) {
		minDistance = dist;
		maxDistance = dist;
	}
	void setLinkDistanceChangePS(float change) {
		linkDistanceChange = change;
	}

	void setMinAngle(double ma) {
		minAngle = ma;
	}
	void setMaxAngle(double ma) {
		maxAngle = ma;
	}
	void setLinkAngle(double ang) {
		minAngle = ang;
		maxAngle = ang;
	}
	void setLinkAngleChangePS(double change, bool rotateSC2) {
		linkRotationChange = change;
		rotateLinkEndSC = rotateSC2;
	}
};