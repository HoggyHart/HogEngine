#pragma once
#include "Src/ECS/components.hpp"
#include <functional>
#include <string>
#include <sstream>
#include "Src/Utils/utils.hpp"
template<typename T>
class TrackerComponent : public EComponent {
private:

	T outputValue{};
	T total{};

	int sampleLimit = 10;
	int currentSamples = 0;
	std::function<T(T&, std::vector<T>&, int&, int&)> updatemethod;

	TextComponent* output = nullptr;
	std::string statTracked = "";
public:
	std::vector<T> trackedValues{};
	//function params: totalValue, recordedValues, sampleLimit,currentSamples
	TrackerComponent(Entity& e, std::string statTracked, std::function<T(T&, std::vector<T>&, int&, int&)> trackMethod) : EComponent(e), statTracked(statTracked), updatemethod(trackMethod) {
	}
	template<typename ...Ts>
	TrackerComponent(Entity& e, std::string statTracked, std::function<T(T&, std::vector<T>&, int&, int&)> trackMethod, Ts ...initialVals) : EComponent(e), statTracked(statTracked), updatemethod(trackMethod) {
		(trackedValues.push_back(initialVals), ...);

		for (T& v : trackedValues) {
			total += v;
		}if (this->getOwner()->hasComponent<TextComponent>()) {
			output = &this->getOwner()->getComponent<TextComponent>();
		}
	}
	//FIX: INSTEAD OF TRACKERCOMPONENT HERE. COULD PROBABLY DO DIFFERENT THING WITH DATACOMP AND FUNCCOMP
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<TrackerComponent>(*this);
	}
	~TrackerComponent() {};
	void fixedUpdate(bool EOI) override {
		outputValue = updatemethod(total, trackedValues, sampleLimit, currentSamples);
		// OR
		std::ostringstream ss;

		if (output) {
			ss << outputValue;
			output->changeText(statTracked + ": " + ss.str());
		}
		else {
			//std::cout << statTracked << ": " << ss.str() << "\n";
		}
	}
	T getTotal() {
		return total;
	}
};