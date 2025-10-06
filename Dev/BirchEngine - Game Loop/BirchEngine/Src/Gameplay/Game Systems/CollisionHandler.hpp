#pragma once
#include <SDL.h>
#include "Src/ECS/ColliderComponent.hpp"
#include "Src/Game Management/ObjectManagement.hpp"
//takes 2 entities, the entity using the method and the entity collided with
//return value indicates whether ignoring the other entity's collision method
//	true: ignore other entity collision method
//	false: let other entity collide
typedef std::function<bool(Entity* E1, Entity* E2)> CollisionCallbackMethod;
typedef CollisionCallbackMethod CCM;

typedef std::string CollisionCallbackID;

struct CollisionHandler {
private:
	static ObjectManager<CollisionCallbackMethod> collisionMethods;
public:
	static std::string registerCollisionMethod(std::string ID, CollisionCallbackMethod& method) {
		return collisionMethods.addObject(ID, &method);
	}
	static CollisionCallbackMethod& getCollisionMethod(std::string ID) {
		return *collisionMethods.getObject(ID);
	}

	//moved from ColliderComponent.cpp. NOT finalised
	//bool linesIntersect(Vector2D& A, Vector2D& B, Vector2D& C, Vector2D& D) {
	//	Vector2D AB = B - A;
	//	Vector2D CD = D - C;
	//	Vector2D CA = A - C;
	//	//check first if CD.x == 0
	//	//if they do then a large skip can be made because CA.x = CD.x*t2 - AB.x*t1  and  CA.y = CD.y*t2 - AB.y*t1 dont need to be simultaneous equation'd
	//	//because then i can just do
	//	float t1 = 0;
	//	float t2 = 0;
	//	float distError = 0.1f;
	//	//these two will return a value
	//	if ((CA.x != 0 && AB.x == CD.x && CD.x == 0) || (CA.y != 0 && AB.y == CD.y && CD.y == 0)) { //if equation like 1 = 0*t2 - 0*t1
	//		return false;
	//	}
	//	else if ((CA.x == 0 && AB.x == CD.x && CD.x == 0) || (CA.y == 0 && AB.y == CD.y && CD.y == 0)) { //i.e. either or both equations are 0 = 0*t2 - 0*t1 meaning parallel lines or both lines are points
	//		if ((A.x + AB.x >= C.x && C.x + CD.x >= A.x)
	//			&& (A.y + AB.y >= C.y && C.y + CD.y >= A.y)) {
	//			return true;
	//		}
	//		else return false;
	//	}
	//	//calculate t1 and t2
	//	if (CD.x == 0 && AB.x != 0) { //i.e. x = 0*t2 - y*t1 --> x = -y*t1
	//		t1 = CA.x / -AB.x;
	//		if (CD.y != 0) {
	//			t2 = (CA.y + AB.y * t1) / CD.y;
	//		}
	//	}
	//	else if (CD.x != 0 && AB.x == 0) {//i.e. x = y*t2 - 0*t1 --> x = y*t2
	//		t2 = CA.x / CD.x;
	//		if (AB.y != 0) {
	//			t1 = (CD.y * t2 - CA.y) / AB.y;
	//		}
	//	}
	//	else if (CD.x != 0 && AB.x != 0) {
	//		t1 = (CA.y - (CA.x * CD.y) / CD.x) / (-AB.y + (AB.x * CD.y) / CD.x);
	//		t2 = (CA.x + AB.x * t1) / CD.x;
	//	}
	//	//t1 and t2 must be between 0 and 1 to make sure they are on the lines AB / CD
	//	if (0 <= t1 && t1 <= 1 && 0 <= t2 && t2 <= 1
	//		&& (A + AB * t1).distance(C + CD * t2) < distError) {
	//		return true;
	//	}
	//	return false;
	//}
	//bool Collision_SqAndSq(CollisionArea* A, CollisionArea* B)
	//{
	//	CollisionAreaSquare* ARect = static_cast<CollisionAreaSquare*>(A);
	//	CollisionAreaSquare* BRect = static_cast<CollisionAreaSquare*>(B);
	//	SDL_Rect& HBA = A->box;
	//	SDL_Rect& HBB = B->box;
	//	//old simple method for non-rotated squares
	//	/*if (HBA.x + HBA.w > HBB.x && HBA.x < HBB.x + HBB.w) {
	//		if (HBA.y + HBA.h > HBB.y && HBA.y < HBB.y + HBB.h) {
	//			return true;
	//		}
	//	}*/
	//	//line intersections
	//	for (int i = 0; i < 4; ++i) {
	//		for (int j = 0; j < 4; ++j) {
	//			if (linesIntersect(ARect->points[i], ARect->points[(i + 1) % 4], BRect->points[j], BRect->points[(j + 1) % 4])) {
	//				return true;
	//			}
	//		}
	//	}
	//	//if lines dont intesect: test if one rect is entirely inside the other rect
	//	//testing center distance // may not be useful here. the limiting may serve no purpose once line intersection has been done
	//	//this is because the only way the lines dont intersect AND the squares collide is if one square is completely inside the other, in which case limiting the distance is pointless as it will HAVE to be within the limit
	//	Vector2D distanceAtoB = Vector2D{ HBB.x + HBB.w / 2,HBB.y + HBB.h / 2 } - Vector2D{ HBA.x + HBA.w / 2,HBA.y + HBA.h / 2 }; //vector from centerA to centerB
	//	Vector2D distanceBtoA = Vector2D{ HBA.x + HBA.w / 2,HBA.y + HBA.h / 2 } - Vector2D{ HBB.x + HBB.w / 2,HBB.y + HBB.h / 2 }; //vector from centerB to centerA
	//	auto& limitDistance = [](Vector2D& D, CollisionArea*& A) {
	//		D.rotate(A->r, nullptr); //rotate for distance limiting to square boundaries
	//		float f = abs(D.x) > abs(D.y) ? abs(A->box.w / 2.0f / D.x) : abs(A->box.h / 2.0f / D.y);
	//		D.x *= f;
	//		D.y *= f;
	//		D.rotate(-A->r, nullptr);
	//		}; //SQUARE LIMIT
	//	limitDistance(distanceAtoB, A);
	//	limitDistance(distanceBtoA, A);
	//	if (distanceAtoB.magnitude() + distanceBtoA.magnitude() >= (Vector2D{ HBB.x + HBB.w / 2,HBB.y + HBB.h / 2 } - Vector2D{ HBA.x + HBA.w / 2,HBA.y + HBA.h / 2 }).magnitude()) return true;
	//	return false;
	//}
	//bool Collision_SqAndCrc(SDL_Rect& A, WHE_Circle& B)
	//{
	//	//THIS CODE DOESNT WORK HOW I INTENDED IT TO BUT IT WORKS PERFECTLY.
	//	SDL_Rect& HBB = B->box;
	//	Vector2D distanceAtoB = Vector2D{ HBB.x + HBB.w / 2,HBB.y + HBB.h / 2 } - Vector2D{ A.x + A.w / 2,A.y + A.h / 2 }; //vector from centerA to centerB
	//	distanceAtoB.rotate(A->r, nullptr); //rotate for distance limiting so square boundaries
	//	distanceAtoB.x = A.w / 2.0f < abs(distanceAtoB.x) ? distanceAtoB.x * A.w / 2.0f / abs(distanceAtoB.x) : distanceAtoB.x; //limit distance by hitbox's bounds
	//	distanceAtoB.y = A.h / 2.0f < abs(distanceAtoB.y) ? distanceAtoB.y * A.h / 2.0f / abs(distanceAtoB.y) : distanceAtoB.y; //limit distance by hitbox's bounds
	//	distanceAtoB.rotate(-Ar, nullptr); //rotate back to simulate rotated square
	//	if ((Vector2D{ A.x + A.w / 2.0f,A.y + A.h / 2.0f } + distanceAtoB).distance({ HBB.x + HBB.w / 2.0f,HBB.y + HBB.h / 2.0f }) <= static_cast<float>(HBB.w) / 2.0f) {
	//		return true;
	//	}
	//	return false;
	//}
	//bool Collision_SqAndArc(CollisionArea* A, CollisionArea* B)
	//{
	//	CollisionAreaSquare* sqA = static_cast<CollisionAreaSquare*>(A);
	//	CollisionAreaArc* arcB = static_cast<CollisionAreaArc*>(B);
	//	SDL_Rect& HBA = A->box;
	//	SDL_Rect& HBB = arcB->box;
	//	Vector2D sqCP = Vector2D{ HBA.x + HBA.w / 2.0f, HBA.y + HBA.h / 2.0f };
	//	Vector2D arcCP = Vector2D{ HBB.x + HBB.w / 2.0f,HBB.y + HBB.h / 2.0f };
	//	//not quite right, doesnt consider actual bounds of Circ hitbox, just distance and angle of centers
	//	Angle th1 = arcB->th1 + arcB->r;
	//	Angle th2 = arcB->th2 + arcB->r;
	//	Vector2D th1Line{ th1 };
	//	Vector2D th2Line{ th2 };
	//	Vector2D th1EP = arcCP + th1Line * HBB.w / 2;
	//	Vector2D th2EP = arcCP + th2Line * HBB.w / 2;
	//	//line coll / problem: if square surrounds arc, it doesnt detect collision AND collision with the arc bit doesnt seem to work
	//	for (int i = 0; i < 4; ++i) {
	//		if (linesIntersect(sqA->points[i], sqA->points[(i + 1) % 4], arcCP, th1EP)) return true;
	//		if (linesIntersect(sqA->points[i], sqA->points[(i + 1) % 4], arcCP, th2EP)) return true;
	//	}
	//	//coll with arc bit
	//	// square & circle collision, but then checkf if the point of collision is in arc range.
	//	Vector2D distanceAtoB = Vector2D{ HBB.x + HBB.w / 2,HBB.y + HBB.h / 2 } - Vector2D{ HBA.x + HBA.w / 2,HBA.y + HBA.h / 2 }; //vector from centerA to centerB
	//	distanceAtoB.rotate(A->r, nullptr); //rotate for distance limiting so square boundaries
	//	distanceAtoB.x = HBA.w / 2.0f < abs(distanceAtoB.x) ? distanceAtoB.x * HBA.w / 2.0f / abs(distanceAtoB.x) : distanceAtoB.x; //limit distance by hitbox's bounds
	//	distanceAtoB.y = HBA.h / 2.0f < abs(distanceAtoB.y) ? distanceAtoB.y * HBA.h / 2.0f / abs(distanceAtoB.y) : distanceAtoB.y; //limit distance by hitbox's bounds
	//	distanceAtoB.rotate(-A->r, nullptr); //rotate back to simulate rotated square
	//	if ((sqCP + distanceAtoB).distance(arcCP) <= static_cast<float>(HBB.w) / 2.0f
	//		&& ((sqCP + distanceAtoB) - arcCP).toAngle().between(arcB->th1 + arcB->r, arcB->th2 + arcB->r)
	//		|| sqCP.distance(arcCP) <= distanceAtoB.magnitude()) { //last line for if arc is inside the square
	//		return true;
	//	}
	//	return false;
	//}
	//bool Collision_CrcAndArc(CollisionArea* A, CollisionArea* B)
	//{
	//	bool drawing = true; //flag for drawing arc points to center of circle hitbox
	//	CollisionAreaArc* arcB = static_cast<CollisionAreaArc*>(B);
	//	SDL_Rect& HBA = A->box;
	//	SDL_Rect& HBB = arcB->box;
	//	Vector2D circCP = Vector2D{ HBA.x + HBA.w / 2.0f, HBA.y + HBA.h / 2.0f };
	//	Vector2D arcCP = Vector2D{ HBB.x + HBB.w / 2.0f,HBB.y + HBB.h / 2.0f };
	//	//not quite right, doesnt consider actual bounds of Circ hitbox, just distance and angle of centers
	//	Vector2D centerDistance = circCP - arcCP;
	//	Angle angleArcToCirc{ (centerDistance).toAngle(),true };
	//	Angle th1 = arcB->th1 + arcB->r;
	//	Angle th2 = arcB->th2 + arcB->r;
	//	Vector2D th1Line{ th1 };
	//	Vector2D th2Line{ th2 };
	//	float d = centerDistance.magnitude();
	//	//std::cout << "drawing back arc\n";
	//	//first check if arc center is within circle area
	//	if (d < HBA.w / 2.0f) return true;
	//	if (d > (HBA.w + HBB.w) / 2.0f) return false; //if the arc and circle couldnt collide even if they were both full circles, return false
	//	//then check if circle center is in arc angle range. If it is then check if their areas overlap. if they dont overlap then they cannot overlap in any other scenario
	//	if (angleArcToCirc.between(th1, th2)) {
	//		if (d < HBA.w / 2.0f + HBB.w / 2.0f) return true;
	//		else return false;
	//	}
	//	// if circle is "behind" arc and arc range <180, check distance to arc center
	//	/*if (angleArcToCirc.between(th2 + Angle{ 0.5 * M_PI ,true }, th1 - Angle{ 0.5 * M_PI ,true })
	//		&& !((arcB->th1 > arcB->th2 && abs(2 * M_PI - (arcB->th2.theta - arcB->th1.theta)) > M_PI)
	//			|| abs(arcB->th2.theta - arcB->th1.theta) > M_PI)) {
	//		return false;*/
	//		//}
	//		//else: shortest distance is to a point on the th1 or th2 lines
	//		//else{
	//	float th1Dist = circCP.distance(arcCP + th1Line * HBB.w / 2.0f); //distance th1EP
	//	float th2Dist = circCP.distance(arcCP + th2Line * HBB.w / 2.0f); //distance to th2EP
	//	bool flip = th1Dist < th2Dist;
	//	//finding closest arc point
	//	Vector2D& relevantLine = flip ? th1Line : th2Line;
	//	Angle closestPointAngle{ flip ? (0.5 * M_PI - centerDistance.toAngle() + th1) : -(0.5 * M_PI - centerDistance.toAngle() + th2), true };
	//	closestPointAngle.bindTo(0, 0.5 * M_PI);
	//	float x = static_cast<float>(d * cos(closestPointAngle));
	//	if (x > HBB.w / 2.0f) x = HBB.w / 2.0f;
	//	else if (x < 0) x = 0;
	//	return circCP.distance(arcCP + (relevantLine * x)) < HBA.w / 2.0f;
	//	//finding line intersection
	//	/*Vector2D& relevantLine = flip ? th1Line : th2Line;
	//	if (circCP.distance(arcCP + relevantLine * HBB.w / 2.0f) < HBA.w / 2.0f) return true;
	//	Vector2D perpendicularAngle = flip ? Vector2D{ th1Line }.rotate(-0.5 * M_PI, nullptr) : Vector2D{ th2Line }.rotate(0.5 * M_PI, nullptr);
	//	perpendicularAngle *= HBA.w / 2.0f;
	//	Vector2D cam{ Game::camera.x,Game::camera.y };
	//	return linesIntersect(arcCP,
	//		arcCP + (relevantLine * HBB.w / 2.0f),
	//		circCP,
	//		circCP + perpendicularAngle);*/
	//		//}
	//	return false;
	//}
	//bool Collision_ArcAndArc(CollisionArea* A, CollisionArea* B)
	//{
	//	SDL_Rect& HBA = A->box;
	//	SDL_Rect& HBB = B->box;
	//	CollisionAreaArc* arcA = static_cast<CollisionAreaArc*>(A);
	//	CollisionAreaArc* arcB = static_cast<CollisionAreaArc*>(B);
	//	return false;
	//}
	static inline bool rectCollision(const SDL_Rect& s1, const SDL_Rect& s2) {
		return (
				s1.x + s1.w >= s2.x
			&&	s1.x <= s2.x + s2.w
			
			&&	s1.y + s1.h >= s2.y
			&&	s1.y <= s2.y + s2.h);
	}
	static inline bool circleCollision(const WHE_Circle& a, const WHE_Circle& b) {
		return a.centre.distance(b.centre) <= a.radius + b.radius;
	}
	static inline bool fancyRectCollision(const WHE_FRect& a, const WHE_FRect& b) {
		return rectCollision(a, b);
	}
	static inline bool doCollide(const CollisionArea& a, const CollisionArea& b) {
		if (a.shape == b.shape) {
			switch (a.shape) {
			case Rectangle:
				return rectCollision(a.rect.hitbox, b.rect.hitbox);
			case Circle:
				return circleCollision(a.circ.area, b.circ.area);
			case FancyRect:
				return fancyRectCollision(a.fRect.area, b.fRect.area);
			}
		}
		return false;
	}
	static inline bool canCollide(const ColliderComponent& a, const ColliderComponent& b) {
		//if either has collisions toggled off globally, they cannot collide
		if (a.ignoresCollisions() || b.ignoresCollisions()) return false;

		//if a has a method for b OR b has a method for a, and these methods arent ignored, they can collide
		return	(a.hasMethodFor(b.getType()) && !b.ignoresType(a.getType()))
			|| (b.hasMethodFor(a.getType()) && !a.ignoresType(b.getType()));
	}
	static inline void processCollision(const ColliderComponent& A, const ColliderComponent& B) {

		//in comments I assume A is priority for quick reading
		//priority is established to allow for dynamic collision ignoring, i.e. Kayn from League of Legends can ignore wall collisions 'sometimes'
		const ColliderComponent& priorityEntity{ A.getCollisionResponseInfo(B.getType()).type > B.getCollisionResponseInfo(A.getType()).type ? A : B};
		const ColliderComponent& lowerEntity{ &priorityEntity == &A ? B : A };

		//create bool to track if A has conditionally avoided the collision method of B (i.e. with Kayn E: Kayn collides to heal + prevents wall from pushing out)
		bool dynamicIgnore = false;

		//if B does not ignore A outright, then perform A's collisionMethod
		if (!lowerEntity.ignoresType(priorityEntity.getType())) {
			CollisionCallbackMethod& collisionMethod = getCollisionMethod(priorityEntity.getCollisionResponseInfo(lowerEntity.getType()).methodID);
			dynamicIgnore = collisionMethod(priorityEntity.getOwner(), lowerEntity.getOwner());
		}
		//if A does not outright or dynamically ignore B, perform method
		if (!dynamicIgnore && !priorityEntity.ignoresType(lowerEntity.getType())) {
			getCollisionMethod(lowerEntity.getCollisionResponseInfo(priorityEntity.getType()).methodID)(lowerEntity.getOwner(), priorityEntity.getOwner());
		}
	}
};

