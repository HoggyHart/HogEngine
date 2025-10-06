#include "Src/ECS/components.hpp"

//turn into Manager so draw can be overriden to use SDL_RenderSetClipRect()
//then create global manager AKA whole window manager that holds containers
//current Manager manager will be set as the "Game entities" manager. // maybe add a HUD manager too? to handle hiding all HUD elements at once, implemented using isDrawing flag across managers

class WindowEntity : public Entity, public Manager {
protected:

	std::map<std::string, Entity*> windowParts{};

	int t = 2;

public:
	std::string windowTitle;

	//WINDOW ENTITY SPAWNS AND ATTACHES TO WRONG POINT (GAME/WINDOW) DEPNDING ON HOW MANY TIMES M1 IS PRESSED. WHAAT??!?!?!!?!
	//WILL ALSO BECOME INCREASINGLY ERRATIC IN ITS PLACEMENT DEPNDING ON M1 CLICKS
	//PROBLEM DUE TO LINK CREATION FROM PLAYER TO SLASHING PROJECTILE.
	//	LINK IS CREATED, BUT WHEN ATTACK FINISHES AND PROJECTILE IS DESTROYED THE LINK IS NOT. MEMORY PROBLEM OR SOMETHING CAUSES THE WINDOW TO BE AFFECTED BY THE LINKS INSTEAD.
	//solutiuon: mark linksOut for deletion in transform component destructor?
	// ^^^ also i think for this i need to add 2way link visibility.
	// ^^^ also this is just a memory leak
	WindowEntity(Manager& m, int x, int y, int w, int h, std::string title) : Entity(m), windowTitle(title){
		static int tbHeight = -1;

		this->addComponent<SpatialComponent>(static_cast<float>(x), static_cast<float>(y), w, h, 1.0f);

		Entity* toolbar = &entityManager.queueAddEntity<Entity>();
		toolbar->addComponent<SpatialComponent>(0.0f, 0.0f, w, tbHeight, 1.0f);
		//need to implement held down button functionality
		toolbar->addComponent<ButtonComponent>([this](bool& pressed, bool beingPressed) {
			static int toolBarGrabX;
			static int toolBarGrabY;
			int x;
			int y;
			SDL_GetMouseState(&x, &y);

			if (beingPressed && !pressed) {
				toolBarGrabX = static_cast<int>(x - this->getComponent<SpatialComponent>().position.x);
				toolBarGrabY = static_cast<int>(y - this->getComponent<SpatialComponent>().position.y);
				pressed = true;
			}
			else if (!beingPressed) {
				pressed = false;
			}

			this->getComponent<SpatialComponent>().position.x = static_cast<float>(x) - toolBarGrabX;
			this->getComponent<SpatialComponent>().position.y = static_cast<float>(y) - toolBarGrabY;
			});
		static SDL_Color black{ 0,0,0,255 };
		toolbar->addComponent<TextComponent>(CENTER_LEFT, title, "Consolasx24", black);
		//toolbar->addComponent<SpriteComponent>("hitbox");
		//add toolbar stuff (button func to drag window around / visual element (blue box for now?))

		Entity* exitButton = &entityManager.queueAddEntity<Entity>();
		exitButton->addComponent<SpatialComponent>(0.0f, 0.0f, tbHeight, tbHeight, 1.0f);
		exitButton->addComponent<ButtonComponent>([this](bool& dm, bool dm2) {
			if (!dm2) this->destroy();
			});
		static SDL_Color white{ 255,255,255,255 };
		//	exitButton->addComponent<SpriteComponent>("bulletPNG");

		Entity* windowArea = &entityManager.queueAddEntity<Entity>();
		windowArea->addComponent<SpatialComponent>(0.0f, 0.0f, w, h, 1.0f);
		//windowArea->addComponent<SpriteComponent>("hitbox");
		windowArea->addComponent<UIComponent>(new GridLayout(5, 5));

		windowParts = { {"tb",toolbar},
			{"eb",exitButton},
			{"wa",windowArea} };
		LinkComponent& l = this->getComponent<SpatialComponent>().addLink(toolbar->getComponent<SpatialComponent>(), std::pair<Vector2D, Vector2D>{ {0, 0 }, { 0,0 } });
		l.setLinkDistance(0.0f);
		l.fixedUpdate(false);

		l = this->getComponent<SpatialComponent>().addLink(exitButton->getComponent<SpatialComponent >(), std::pair<Vector2D, Vector2D>{ {w - tbHeight, 0}, { 0,0 } });
		l.setLinkDistance(0.0f);
		l.fixedUpdate(false);

		l = this->getComponent<SpatialComponent>().addLink(windowArea->getComponent<SpatialComponent>(), std::pair<Vector2D, Vector2D>{{ 0, tbHeight }, { 0,0 }});
		l.setLinkDistance(0.0f);
		l.fixedUpdate(false);

		this->addGroup(EntityGroup::groupGameHUD);
		this->addGroup(EntityGroup::groupWindows);
	}

	~WindowEntity() {
		for (std::pair<const std::string, Entity*>& p : windowParts) {
			p.second->destroy();
		}
		//handle contained stuff too. Not sure if has to be careful about perma deleting stuff i.e.
		// in inventory screen when deleting window make sure to delete contained item entities in a way that doesnt delete the items from the inventory / the game.
	}

	void addEntity(Entity& e) {
		windowParts["wa"]->getComponent<UIComponent>().addToContainer(e);
		e.clearGroups();
	}
	void clearWindow() {
		//POTFIX: true is passed to destroy all the entities contained in the window. this is assuming that the window has complete control
		//		over the entities inside it.
		windowParts["wa"]->getComponent<UIComponent>().clear(true);
	}

	void draw() {
		for (auto& c : components) c->draw(nullptr);
		SDL_Rect area{};
		SpatialComponent* tf = &windowParts["tb"]->getComponent<SpatialComponent>();
		tf->toRect(area);
		SDL_SetRenderDrawColor(Game::renderer, 200, 200, 255, 255);
		SDL_RenderFillRect(Game::renderer, &area);
		tf = &windowParts["eb"]->getComponent<SpatialComponent>();
		tf->toRect(area);

		SDL_SetRenderDrawColor(Game::renderer, 255, 50, 50, 255);
		SDL_RenderFillRect(Game::renderer, &area);
		tf = &windowParts["wa"]->getComponent<SpatialComponent>();
		tf->toRect(area);

		SDL_SetRenderDrawColor(Game::renderer, 225, 225, 225, 255);
		SDL_RenderFillRect(Game::renderer, &area);

		for (auto& p : windowParts) p.second->draw(nullptr);
	}
};