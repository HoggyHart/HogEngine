#pragma once
#include "components.hpp"
#include "Src/Gameplay/UserInterface/PopupBox.hpp"

typedef std::function<void(Entity* interactableEntity, Entity* interacter, Uint8& interactionStage)> InteractionMethod;
static const std::function<void(Entity* interactableEntity, Entity* interacter, Uint8& interactionStage)> defaultInteraction{ [](Entity* a, Entity* b, Uint8& c) {std::cout << "Interacted with Entity\n"; } };

class InteractableComponent : public EComponent {
private:
	//PLACEHOLDER POSITION. POSSIBLY BETTER PLACE TO PUT THIS?

	bool displayingPrompt{ false };

	InteractionMethod method{ defaultInteraction };
	Uint8 interactionStage{ 0 }; //used in interactionMethod to determine what to do when interacted with.
	// (i.e. a chest has interaction stage 0 (closed) and stage 1 (open), when at stage 0, interaction will open the chest, at stage 1 it will close the chest
	//
	Entity* interactablePopup{ nullptr };
public:
	InteractableComponent(Entity& e) : EComponent(e) {
		e.addGroup(EntityGroup::groupInteractables);
	};
	InteractableComponent(Entity& e, InteractableComponent& i) : EComponent(e),
		method(i.method) {
		e.addGroup(EntityGroup::groupInteractables);
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<InteractableComponent>(*this);
	}
	~InteractableComponent() {
		//NOTE: last time i saw this i questioned it since popup is dependant on entity anway.
		//note that this component can be removed before the entity is destroyed. and if done needs the popup to be destroyed alogn with it
		if (interactablePopup) interactablePopup->destroy();
		this->getOwner()->delGroup(EntityGroup::groupInteractables);
	}
	void setInteractionResult(InteractionMethod& im) { method = im; }
	void fixedUpdate(bool EOI) override {
#pragma region interactionPromptCode

		bool playersCloseEnough = false;
		SpatialComponent& sc = this->getOwner()->getComponent<SpatialComponent>();
		//check if needed to spawn a prompt
		for (Entity* e : this->getOwner()->entityManager.getGroup(EntityGroup::groupPlayers)) {
			if (e->getComponent<SpatialComponent>().getCenter().distance(sc.getCenter()) < sc.width * sc.scale * 1.5) { //sc.width... should prob be changed to a bounding box. maybe slightly larger than the SC?
				playersCloseEnough = true;
				if (!interactablePopup) {
					interactablePopup = &this->getOwner()->entityManager.queueAddEntity<HighlightFrameEntity>(static_cast<int>(sc.position.x), static_cast<int>(sc.position.y), sc.width, sc.height, true);
					this->getOwner()->addDependant(interactablePopup);
					return;
				}
			}
		}
		//check if needed to remove prompt
		if (!playersCloseEnough && interactablePopup) {
			interactablePopup->destroy();
			interactablePopup = nullptr;
		}
#pragma endregion
	}
	void interact(Entity* interacter) {
		//if prompt is not to be shown after interaction, it can be accessed through this->entity OR it can be destroyed by destroying the interactable component
		method(this->getOwner(), interacter, interactionStage);
	}
};