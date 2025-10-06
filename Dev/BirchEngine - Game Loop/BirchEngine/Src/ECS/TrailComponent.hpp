#include "components.hpp"
class TrailComponent : public EComponent {
public:
	Uint32 dur;
	Uint32 trailInterval;

	Uint32 lastTrailEntityTime{ 0 };

	//POTFIX: trailVector currently used for initialisation of duration component. is there a better way?
	// adding DurComp to entity in interpolatedUpdate() causes interpolation loop in draw() to invalidate the iterator
	std::vector<Entity*> trailEntities;

	TrailComponent(Entity& e, Uint32 trailDuration, Uint32 trailInterval) : EComponent(e), dur(trailDuration), trailInterval(trailInterval) {
		e.addGroup(EntityGroup::groupInterpolatables);
	}
	TrailComponent(Entity& e, TrailComponent& tc) :
		EComponent(e),
		dur(tc.dur),
		trailInterval(tc.trailInterval),
		lastTrailEntityTime{ 0 },
		trailEntities{} {
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<TrailComponent>(*this);
	}
	~TrailComponent() {
		//FIX: If removing the TrailComponent from an entity due to loss of speed
		// deleting their interpolation group may be bad because they may be interpolatable for other reasons
		// ideally i think trailcomponent shouldnt be a component thing but more an effect added by a system/other entity for simplicity
		// 
		this->getOwner()->delGroup(EntityGroup::groupInterpolatables);
	}
	void init() override {
		this->getOwner()->reorderComponents({ this,&this->getOwner()->getComponent<SpriteComponent>() });
	}
	void fixedUpdate(bool EOI) override {
		for (auto& p : trailEntities) {
			p->addComponent<DurationComponent>(dur);
		}
		trailEntities.clear();
	}
	void interUpdate() override {
		if (GameTime::totalTime - lastTrailEntityTime >= trailInterval) {
			trailEntities.push_back(&this->getOwner()->entityManager.queueAddEntity<Entity>());
			Entity* e = trailEntities.back();
			e->addGroup(EntityGroup::groupWeaponEffects);
			SpatialComponent& spc = e->addComponent<SpatialComponent>(this->getOwner()->getComponent<SpatialComponent>());
			SpriteComponent& sc = e->addComponent<SpriteComponent>(this->getOwner()->getComponent<SpriteComponent>());
			FadeAwayComponent& fc = e->addComponent<FadeAwayComponent>(dur);
			fc.remainingFadeDuration = dur * 0.75;
			sc.animated = false;
			lastTrailEntityTime = GameTime::totalTime;
		}
	}
};