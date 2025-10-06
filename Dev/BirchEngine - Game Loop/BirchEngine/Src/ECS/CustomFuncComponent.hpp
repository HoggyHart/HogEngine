#pragma once
#include "ECS.hpp"
#include <functional>

class CustomFunc : public EComponent {
	std::function<void()> func;
public:
	CustomFunc(Entity& e, std::function<void()> func) : EComponent(e), func(func) {
	}
	CustomFunc(Entity& e, CustomFunc& cf) : EComponent(e), func(cf.func) {}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<CustomFunc>(*this);
	}
	void fixedUpdate(bool EOI) override {
		func();
	}
};