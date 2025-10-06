#pragma once
#include "Src/Gameplay/CreatureStuff/LimitedResource.hpp"
#include "Src/ECS/ECS.hpp"
#include "Src/Game Management/GameTime.hpp"
class ResourcesComponent : public EComponent {
private:
	std::map<ResourceType, std::vector<Resource>> resources;

public:

	ResourcesComponent(Entity& e) : EComponent(e) {}
	ResourcesComponent(Entity& e, ResourcesComponent& rc) : EComponent(e), resources(rc.resources) {}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<ResourcesComponent>(*this);
	}
	void fixedUpdate(bool EOI) override {
		for (std::pair<const ResourceType, std::vector<Resource>>& resourceGroup : resources) {
			for (Resource& rsrc : resourceGroup.second) {
				rsrc.regenerate(GameTime::fixedDeltaTime);
			}
		}
	}
	void addResource(ResourceType t, Resource rsrc) {
		//FIX: order of resources specification. might need to just be managed when resources are set for a creature
		//currently, new resources are added to the front of the vector, so if shield was added it would come before health.
		//	maybe a problem with: if fuel is the only resource, but then BACKUP fuel is added, it would come BEFORE the regular fuel.
		resources[t].emplace(resources[t].begin(), rsrc);
	}
	void removeResource(std::string resourceName) {
		//FIX: implement method
	}
	std::vector<Resource>& getResources(ResourceType t) {
		return resources[t];
	}
	Resource& getResource(std::string name) {
		for (std::pair<const ResourceType, std::vector<Resource>>& resourceGroup : resources) {
			for (Resource& rsrc : resourceGroup.second) {
				if (rsrc.resourceName == name) return rsrc;
			}
		}
	}
};