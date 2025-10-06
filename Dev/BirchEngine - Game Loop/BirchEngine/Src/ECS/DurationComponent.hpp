#pragma once
#include "Src/ECS/ECS.hpp"
#include "Src/Game Management/GameTime.hpp"
//CONTINUE: could be used for setting stuff up to do osmething after X seconds
class DurationComponent : public EComponent {
	int originalDuration;
	int timeLeft;

public:
	DurationComponent(Entity& e, Uint32 timeDuration) : EComponent(e),
		originalDuration(timeDuration),
		timeLeft(timeDuration) {
		e.addGroup(EntityGroup::groupInterpolatables);
	}
	DurationComponent(Entity& e, DurationComponent& dc) : EComponent(e),
		originalDuration(dc.originalDuration),
		timeLeft(originalDuration) {
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<DurationComponent>(*this);
	}
	~DurationComponent() {
	}
	void fixedUpdate(bool EOI) override {
		if (timeLeft < 0) this->getOwner()->destroy();
	}
	void interUpdate() override {
		timeLeft -= GameTime::deltaTime;
	}
};