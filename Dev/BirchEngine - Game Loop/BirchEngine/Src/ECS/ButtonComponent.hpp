#include "components.hpp"
#include <functional>

//ButtonMethod takes bool& that tracks if button is activated.
//this is because some buttons activated while they are held, and others are activated after 1 press, then deactivated after another
//atm (17/11/2024) im thinking the button method will handle whether it stays pressed or not.
//circumstance: the button is pressed -> if another button has been pressed, this stays pressed
//									  -> if another button not been pressed, deactivate this button.
//special cases like this require bool& to be passed

//2nd bool is to indicate being pressed (true), or being released(false)
typedef std::function<void(bool&, int)> ButtonMethod;

class ButtonComponent : public EComponent {
private:
	ButtonMethod onClick;
	ButtonMethod onHover;
	bool activated{ false };
public:
	SpatialComponent* tf = nullptr;
	ButtonComponent(Entity& e, ButtonMethod f) : EComponent(e), onClick(f) {
		this->getOwner()->addGroup(EntityGroup::groupButtons);
		if (this->getOwner()->hasComponent<SpatialComponent>()) {
			tf = &this->getOwner()->getComponent<SpatialComponent>();
		}
	};
	ButtonComponent(Entity& e, ButtonComponent& bc) : EComponent(e),
		onClick(bc.onClick),
		onHover(bc.onHover) {
	}

	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<ButtonComponent>(*this);
	}
	~ButtonComponent() {
		delete tf;
		this->getOwner()->delGroup(EntityGroup::groupButtons);
	}
	void fixedUpdate(bool E) override {
		if (activated) {
			onClick(activated, true);
		}
	}
	void setOnClick(ButtonMethod f) {
		this->onClick = f;
	}
	void clicked(bool beingPressed) {
		onClick(activated, beingPressed);
	}

	void setOnHover(ButtonMethod f) {
		onHover = f;
	}
	void hovered(bool beingHovered) {
		onClick(activated, beingHovered);
	}
};