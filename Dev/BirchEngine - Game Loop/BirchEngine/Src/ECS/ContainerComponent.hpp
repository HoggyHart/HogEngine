#pragma once
#include "components.hpp"
#include <vector>

//organisation pattern
enum design { LayoutStyle_Random, LayoutStyle_Grid };

struct Layout {
	SDL_Rect* layoutArea = nullptr;
	Layout() {};

	virtual void updateLayout(std::vector<std::vector<Entity*>>& entities, SDL_Rect& bounds) {};
	virtual void addToLayout(Entity* entity) {};
	virtual void addToLayout(Entity* e, int row, int col) {};
};
struct GridLayout : public Layout {
	//basic grid
	int cols;
	int rows;

	//bonus params
	int xPadding = 0;
	int yPadding = 0;
	int xSpacing = 0;
	int ySpacing = 0;
	//more params here (i.e. shrink entities so they fit entirely in their grid slot?
	//						or let them exceed their grid slot?
	//						or let them stay full size but clip to grid slot?

	GridLayout(int cols, int rows) :cols{ cols }, rows{ rows } {
	}
	void updateLayout(std::vector<std::vector<Entity*>>& entities, SDL_Rect& bounds) override {
		int eW = (bounds.w - xPadding - xSpacing * (cols - 1)) / (cols);
		int eH = (bounds.h - yPadding - ySpacing * (rows - 1)) / (rows);
		int x = xPadding / 2;
		int y = yPadding / 2;

		for (std::vector<Entity*>& row : entities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					SpatialComponent& tf = entity->getComponent<SpatialComponent>();
					tf.position.x = static_cast<float>(x + bounds.x);
					tf.position.y = static_cast<float>(y + bounds.y);
					// stupid VVV
					tf.width = eW;
					tf.height = eH;
					//stupid ^^^
				}
				x += eW + xSpacing;
			}
			x = xPadding / 2;
			y += eH + ySpacing;
		}
	}
};
struct FreeLayout : public Layout {
private:

public:
	int entityLimit = -1;

	FreeLayout() {}; //no params means no entity limit, and layout area takes up whole container

	FreeLayout(int x) :entityLimit(x) {};
};

//this is more like a LayoutComponent

struct ContainerBounds {
	float& x;
	float& y;
	int& w;
	int& h;
	float& s;
};
class UIComponent : public EComponent {
private:
	void commonAddToContainerProcesses(Entity* e) {
		//resize scale so that when changing width and height to fit (in layout ->updateLayout()) it fits as intended
		e->getComponent<SpatialComponent>().scale = 1.0f;

		if (this->getOwner()->isGameEntity()) e->makeGameEntity();
		else e->makeDisplayEntity();

		for (std::size_t g = drawGroupsSaE.first; g <= drawGroupsSaE.second; ++g) {
			if (e->hasGroup(g)) e->delGroup(g);
		}
	}
public:
	//FIX: MAKE LAYOUT A UNION
	Layout* layout = nullptr;
	std::vector<std::vector<Entity*>> containedEntities;
	int containerSize = 0;
	SpatialComponent* etf = nullptr;
	SDL_Rect containerBounds{};
public:

	UIComponent(Entity& e, GridLayout grid) : EComponent(e), containedEntities(grid.rows > 0 ? grid.rows : 1, std::vector<Entity*>(grid.cols, nullptr)), containerSize(grid.rows* grid.cols) {
		layout = new GridLayout(grid);
	};
	UIComponent(Entity& e, FreeLayout free) : EComponent(e), containedEntities(1, std::vector<Entity*>(free.entityLimit > -1 ? free.entityLimit : 0, nullptr)), containerSize(free.entityLimit) {
		layout = new FreeLayout(free);
	};
	UIComponent(Entity& e, UIComponent& cc) : EComponent(e),
		layout(cc.layout),
		containerSize(cc.containerSize),
		containedEntities(cc.containedEntities),
		etf(&this->getOwner()->getComponent<SpatialComponent>()),
		containerBounds(cc.containerBounds)

	{
		//and turn all the entities stored into copies of the original
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					entity = new Entity(*entity);
				}
			}
		}
	}
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<UIComponent>(*this);
	}
	~UIComponent() {
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					entity->destroy();
				}
			}
		}
	}

	Entity*& getNextEmptySlot() {
		//look across each right left to right to find next clear slot
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				//if slot clear, fill it
				if (entity == nullptr) {
					return entity;
				}
			}
		}
	}
	Entity*& addToContainer(Entity* e) {
		commonAddToContainerProcesses(e);
		//if container has set entity limit
		if (containerSize > -1) {
			//look across each right left to right to find next clear slot
			for (std::vector<Entity*>& row : containedEntities) {
				for (Entity*& entity : row) {
					//if slot clear, fill it
					if (entity == nullptr) {
						entity = e;
						return entity;
					}
				}
			}
		}
		else {
			for (Entity*& entity : containedEntities[0]) {
				if (entity == nullptr) {
					entity = e;
					return entity;
				}
			}
			containedEntities[0].push_back(e);
		}
		this->fixedUpdate(false);
		return e;
	}
	Entity*& addToContainer(Entity* e, int row, int col) {
		commonAddToContainerProcesses(e);
		containedEntities[row][col] = e;
		this->fixedUpdate(false);
		return containedEntities[row][col];
	};

	void clear(bool destroy) {
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					if (destroy) entity->destroy();
					entity = nullptr;
				}
			}
		}
	}
	inline void updateBounds() {
		containerBounds.x = static_cast<int>(etf->position.x);
		containerBounds.y = static_cast<int>(etf->position.y);
		containerBounds.w = static_cast<int>(etf->width * etf->scale);
		containerBounds.h = static_cast<int>(etf->height * etf->scale);
	}
	void init() override {
		etf = &this->getOwner()->getComponent<SpatialComponent>();
		updateBounds();
	}

	void fixedUpdate(bool EOI) override {
		updateBounds();
		layout->updateLayout(containedEntities, containerBounds);
	}

	void draw() override {
		//containerBounds is relative to the window
		//entity->draw() will be relative to the in-game world/camera
		//solution: draw() takes a parameter SDL_Rect* indicating which camera to be relative to, if SDL_Rect* == nullptr, just draw to window
		SDL_RenderSetClipRect(Game::renderer, &containerBounds);
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					entity->draw();
				}
			}
		}
		SDL_RenderSetClipRect(Game::renderer, nullptr);
	}
	void draw(ViewPort* camera) override {
		//containerBounds is relative to the window
		//entity->draw() will be relative to the in-game world/camera
		//solution: draw() takes a parameter SDL_Rect* indicating which camera to be relative to, if SDL_Rect* == nullptr, just draw to window
		SDL_RenderSetClipRect(Game::renderer, &containerBounds);
		for (std::vector<Entity*>& row : containedEntities) {
			for (Entity*& entity : row) {
				if (entity != nullptr) {
					entity->draw(camera);
				}
			}
		}
		SDL_RenderSetClipRect(Game::renderer, nullptr);
	}
};