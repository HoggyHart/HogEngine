#include "GameEvents.hpp"
#include "Src/Game Management/Game.hpp"
#include "Src/Gameplay/CreatureStuff/LimitedResource.hpp"
#include "Src/ECS/ResourcesComponent.hpp"
std::queue<WHE_Event> gameEventQueue{};

void finaliseEvent(WHE_Event& ev)
{
	switch (ev.type) {
	case WHE_DamageEvent:
	{
		DamageEvent& dmgEvent = ev.event.damage;
		//POTFIX: consider how modifiers will work with the overkill damageEvent. the total is what gets reduced, but the baseValue and modiifers will be the same as it currently is.
		//		if a reaction to the carry-over event would affect the modifiers, what happens ? with just "ignore bonus damage (i.e. beyond the base damage)" it could be as simple as using base instead of total, but what if THAT carries over too?
		//	i.e. shield takes X damage, health takes the remainder, Y, and has an effect which eliminates any
		// it probably wont be a problem and im overthinking about nothing.

		float& damage = dmgEvent.remainingDamage.amount.baseValue;
		std::vector<Resource>& hitPoints = dmgEvent.damagedEntity->getComponent<ResourcesComponent>().getResources(rsrcHitPoints);
		std::vector<Resource>::iterator it = hitPoints.begin();
		while (it != hitPoints.end()) {
			//deal damage if this hp stat is not depleted
			if (it->cur > 0) it->cur -= damage;
			//if it is already depleted then skip to next hp stat
			else {
				++it;
				continue;
			}

			//if damage dealt and hitpoints are now fully depleted
			if (it->cur <= 0) {
				damage = -it->cur; //totalDamage = carryOver damage; i.e. remainder after reducing hp to 0
				it->cur = 0;

				//if depleted this hp and there is none after it. DEATH FINISH
				if (it + 1 == hitPoints.end()) {
					WHE_Event& newEv = WHE_QueueEvent(WHE_EntityDeath);
					new (&newEv.event.death) DeathEvent{ dmgEvent.damagedEntity,dmgEvent.damagerEntity,dmgEvent };

					return;
				}

				//if more hp remaining but no more damage. FINISH
				else if (damage == 0) return;

				//if more hp AND damage to carry over, do a DAMAGE CARRY FINISH
				else {
					//set event as carry over and re-add it to the queue
					dmgEvent.carryOverEvent = true;
					WHE_Event& newEv = WHE_QueueEvent(WHE_DamageEvent);
					new (&newEv.event.damage) DamageEvent{ dmgEvent };
					return;
				}
			}
			//if damage was not enough to deplete hp: event is finished
			else return;
		}
		//if escaped loop it means no damage was dealt at all
		//	this could mean A) damage was 0 or B) no hitpoints remaining
		//	either way this means nothing happens as entity was already dead or damage was 0
		//	so just end function
		return;
	}
	case WHE_EntityDeath:
		//CONTINUE: left off here, just put a marker to indcate end of case result.
		ev.event.death.victim->destroy();
		break;
	}
}

DamageEvent::DamageEvent(Entity* victim, Entity* damager, DamageStats damage)
	: type(WHE_DamageEvent), damagedEntity(victim), damagerEntity(damager), timestamp(GameTime::totalTime), originalDamage(damage), remainingDamage(damage), carryOverEvent(false)
{
	//so that effects on the total damage can occur / damage to be processed is just a flat amount
	remainingDamage.amount.baseValue = remainingDamage.recalculateRawDamage(victim->getComponent<ResourcesComponent>().getResource("Health"));
	remainingDamage.amount.resetMods();
}