#pragma once

#include <vector>
#include <iostream>
#include <array>
#include <memory>
#include <algorithm>
#include <bitset>
class Entity;
class Manager;
typedef unsigned int Uint32;
struct ViewPort;
using ComponentID = std::size_t;
using Group = std::size_t;

template<typename ComponentOwnerType>
class Component {
private:
	//pointer to entityManager of the component
	ComponentOwnerType* owner{ nullptr };
public:
	Component(ComponentOwnerType& e) : owner(&e) {};
	virtual Component<ComponentOwnerType>* copyTo(ComponentOwnerType& e) { throw; };
	inline ComponentOwnerType* getOwner() const { return owner; }
	virtual ~Component() {}
};//creates a static ComponentID (std::size_t) value when the method is called. this value increases each time the method is called.

template<typename ComponentOwnerType>
inline ComponentID getNewComponentTypeID() {
	static ComponentID lastID = 0u;
	return lastID++;
}

//creates a static ComponentID value for each template T (i.e. for int, string, float, etc.)
template <typename ComponentOwnerType, typename T>
inline ComponentID getNewComponentTypeID() noexcept {
	static_assert(std::is_base_of<Component<ComponentOwnerType>, T>::value, "");
	static ComponentID typeID = getNewComponentTypeID<ComponentOwnerType>();
	return typeID;
}

constexpr std::size_t maxComponents = 32;
constexpr std::size_t maxGroups = 32;
using ComponentBitSet = std::bitset<maxComponents>;
using GroupBitSet = std::bitset<maxGroups>;

//template decides what kind of components to use, i.e. EntityComponent or ItemComponent
template<
	typename ComponentType, typename ComponentOwnerType,
	typename std::enable_if_t<std::is_base_of<Component<ComponentOwnerType>, ComponentType>::value, int> = 0 >
struct ComponentSystem {
protected:
	std::vector<std::unique_ptr<ComponentType>> components{}; //contains entity components

	std::array<ComponentType*, maxComponents> componentArray{}; //contains components entity uses
	ComponentBitSet componentBitSet{};
public:

	template <typename T,
		typename std::enable_if_t<std::is_base_of<ComponentType, T>::value, int> = 0 >
	bool hasComponent() const {
		return componentBitSet[getNewComponentTypeID<ComponentOwnerType, T>()];
	}

	template <typename T, typename ...TArgs,
		typename std::enable_if_t<std::is_base_of<ComponentType, T>::value, int> = 0 >
	T& addComponent(TArgs&&... mArgs) {
		if (this->hasComponent<T>()) { std::cout << "ENTITY ALREADY HAS THIS COMPONENT"; throw; }
		T* c(new T(*static_cast<ComponentOwnerType*>(this), std::forward<TArgs>(mArgs)...));
		//c->entity = this;
		std::unique_ptr<ComponentType> uPtr{ c };

		components.emplace_back(std::move(uPtr));
		componentArray[getNewComponentTypeID<ComponentOwnerType, T>()] = c; //if it does not already have a component of this type, register them as the "prime" components of this type (i.e. the type accessed by getComponent
		componentBitSet[getNewComponentTypeID<ComponentOwnerType, T>()] = true;
		c->init();
		return *c;
	}

	Component<ComponentOwnerType>* addComponent(Component<ComponentOwnerType>* component) {
		//FIX/CONTINUE: FINISH ADDING COPYTO() OVERRIDES
		if (component == nullptr) { std::cout << "COPYING NULL COMPONENT"; throw; }
		else return component->copyTo(*static_cast<ComponentOwnerType*>(this));
	}
	template <typename T,
		typename std::enable_if_t<std::is_base_of<ComponentType, T>::value, int> = 0 >
	T& getComponent() const {
		if (!hasComponent<T>()) throw;
		ComponentType* ptr(componentArray[getNewComponentTypeID<ComponentOwnerType, T>()]);
		return *static_cast<T*>(ptr);
	}

	template<typename T,
		typename std::enable_if_t<std::is_base_of<ComponentType, T>::value, int> = 0 >
	void delComponent() {
		if (this->hasComponent<T>()) {
			components.erase(
				std::remove_if(
					components.begin(),
					components.end(),
					[&](std::unique_ptr<ComponentType>& c) {
						if (c.get() == static_cast<ComponentType*>(&this->getComponent<T>())) return true;
						return false;
					}
				),
				components.end()
			);
			componentArray[getNewComponentTypeID<ComponentOwnerType, T>()] = nullptr;
			componentBitSet[getNewComponentTypeID<ComponentOwnerType, T>()] = false;
		}
	}

	//1. changes order of components in vector
	//  component B may depend on component A already existing, so A must be added before B
	//	but if component B should affect A (i.e. transparency for a sprite component) they must be reordered
	//2. array stays the same, which reflects the order in which components were added
	void reorderComponents(std::vector<ComponentType*> cs) {
		//first verify the components provided are part of this entity
		for (ComponentType*& c : cs) {
			if (std::find_if(components.begin(), components.end(), [c](std::unique_ptr<ComponentType>& comp) {return c == comp.get(); }) == components.end()) return;
		}

		//then reorder
		for (size_t i = 0; i < cs.size() - 1; ++i) {
			std::vector<std::unique_ptr<ComponentType>>::iterator it = std::find_if(components.begin(), components.end(), [&cs, &i](std::unique_ptr<ComponentType>& c) {return c.get() == cs[i]; });
			ComponentType* comp = it->get(); it->release();
			components.erase(it); //remove value from vector
			components.insert(std::find_if(components.begin(), components.end(), [cs](std::unique_ptr<ComponentType>& comp) {return cs.back() == comp.get(); }), std::unique_ptr<ComponentType>(comp));
		}
	}
};