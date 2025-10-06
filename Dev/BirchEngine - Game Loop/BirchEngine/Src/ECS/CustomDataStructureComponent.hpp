#pragma once
#include "Src/ECS/ECS.hpp"

template<typename T>
class DataComponent : public EComponent {
public:

	T data;

	template <typename ...DataArgs>
	DataComponent(Entity& e, DataArgs ...args) : EComponent(e), data{ args... } {
	};
	DataComponent(Entity& e, DataComponent<T>& dc) : EComponent(e), data(dc.data) {}

	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<DataComponent>(*this);
	}
};