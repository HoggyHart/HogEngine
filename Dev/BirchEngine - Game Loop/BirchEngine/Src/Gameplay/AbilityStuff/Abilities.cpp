#include "Abilities.hpp"

//ALL UNION STUFF THAT I UNDID AND ACCIDENTALLY PREVENTED REDOING WITH CTRL+Y
//#include "AbilitySetComponent.hpp"
///* ACTIVE ACTIVATION METHODS */
//void ActiveActivation::initActivationDetection()
//{
//	//ability entityManager is usually the ItemStackEntity;
//	Entity*& abilityOwner = abilityToActivate->restOfSet->abilitySetOwner;
//	//if entityManager has no inputcontroller, add one
//	if (!abilityOwner->hasComponent<InputController>()) abilityOwner->addComponent<InputController>();
//
//	InputResponseMethod methodActivation{ [this](Entity* e, bool& methodActive, bool& comboActive) {this->abilityToActivate->method(*this->abilityToActivate); } };
//	Control control{ InputResponse{methodActivation,false},KeyComboInput{{activationButton},true} };
//
//	irm = abilityOwner->getComponent<InputController>().addControl(control).getID();
//
//	detectionInited = true;
//	detectionActive = true;
//}
//
//void ActiveActivation::deInitActivationDetection()
//{
//	//if not needed to be deInited via a lack of InputController to deInit from
//	if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
//		detectionInited = false; return;
//	}
//	abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().delControl(irm);
//	detectionInited = false;
//	detectionActive = false;
//}
//
//void ActiveActivation::blockDetection()
//{
//	if (!detectionActive) return; //if already blocked, skip
//	if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
//		detectionActive = false;
//		return;
//	}
//	abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().blockControl(irm);
//	detectionActive = false;
//}
//
//void ActiveActivation::unblockDetection()
//{
//	if (detectionActive) return; //if already unblocked, skip
//	if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<InputController>()) {
//		//FIX: maybe initActivation()? might not be intended? idk
//		detectionActive = false;
//		return;
//	}
//	abilityToActivate->restOfSet->abilitySetOwner->getComponent<InputController>().unblockControl(irm);
//	detectionActive = true;
//}
///*****************************/
//
//
//
///* PASSIVE ACTIVATION METHODS */
//void PassiveActivation::initActivationDetection()
//{
//	//ability entityManager is usually the ItemStackEntity;
//	Entity*& abilityOwner = abilityToActivate->restOfSet->abilitySetOwner;
//	//if entityManager has no inputcontroller, add one
//	if (!abilityOwner->hasComponent<GameEventListenerComponent>()) abilityOwner->addComponent<GameEventListenerComponent>();
//
//	EventResponseMethod methodActivation{ [this](Entity* e, WHE_Event ev, bool& methodActive) {this->lastProcInfo = ev; this->abilityToActivate->method(*this->abilityToActivate); } };
//	ResponseControl control{ EventResponse{methodActivation,activationEvent == WHE_NoEvent},activationEvent,deActivationEvent };
//	erm = abilityOwner->getComponent<GameEventListenerComponent>().addControl(control).getID();
//
//	detectionInited = true;
//	detectionActive = true;
//}
//
//void PassiveActivation::deInitActivationDetection()
//{
//	//if not needed to be deInited via a lack of InputController to deInit from
//	if (!abilityToActivate->restOfSet->abilitySetOwner->hasComponent<GameEventListenerComponent>()) {
//		detectionInited = false; return;
//	}
//	abilityToActivate->restOfSet->abilitySetOwner->getComponent<GameEventListenerComponent>().delControl(erm);
//	detectionInited = false;
//	detectionActive = false;
//}
//void PassiveActivation::blockDetection()
//{
//}
//void PassiveActivation::unblockDetection()
//{
//}
///******************************/

void Ability::setActivationMethod(ActivationMethod* method)
{
	if (activationCriteria != nullptr) delete activationCriteria;
	switch (method->type) {
	case atActive:
		activationCriteria = new ActiveActivationMethod(*static_cast<ActiveActivationMethod*>(method));
		break;
	case atPassive:
		activationCriteria = new PassiveActivationMethod(*static_cast<PassiveActivationMethod*>(method));
		break;
	default:
		throw;
	}

	activationCriteria->abilityToActivate = this;
}