#pragma once
#include "Src/ECS/ECS.hpp"
#include "Src/ECS/SpatialComponent.hpp"
#include "Src/ECS/CustomDataStructureComponent.hpp"
#include "Src/Gameplay/UserInterface/InterfaceData/UI_Element_Info.hpp"
#include <SDL.h>

//FIX/CONTINUE: UI_ELEMENT_INFO should dictate DESIRED sizes n stuff. SpatialComponent will contain actual. 
// i.e. for different width desires in a column, widest will be used but smallest should be remembered if the wider one is removed
// check that UI_ELEMENT_INFO is being inspected when calculating size hints and stuff

//FIX: also figure out removing destroyed entities from the grid
//	SOLUTION/VERIFY: entities in a grid must have a UIComponent which will contain parent.
//					when entity is destroyed it will go to its parent and remove itself from it

//
class UIComponent;

class Layout {
protected:
	SpatialComponent* gridBounds{ nullptr }; //FIX: this will be traded for a UIComponent that will hold the parent
public:
	SpatialComponent* getBounds() { return gridBounds; }
	void setBounds(SpatialComponent& b) { gridBounds = &b; }
	Layout() {};
	~Layout() {};


	virtual bool getNextEmptySlot(Entity*& store) { throw; };

	virtual void reallignLayout() { throw; };
	
	virtual std::vector<Entity*> getContents() { throw; }

	virtual bool addToLayout(Entity& entity, UI_Element_Info& emplaceInfo) { throw; };
	virtual void removeFromLayout(Entity* entity) { throw; }
	virtual void emptyLayout(bool destroy) { throw; }
};



class UIComponent : public EComponent {
private:
	void commonAddToContainerProcesses(Entity* e) {
		//resize scale so that when changing width and height to fit (in layout ->updateLayout()) it fits as intended
		e->getComponent<SpatialComponent>().scale = 1.0f;

		if (this->getOwner()->isGameEntity()) e->makeGameEntity();
		else e->makeDisplayEntity();

		for (std::size_t g = DRAW_GROUPS_START; g <= DRAW_GROUPS_END; ++g) {
			if (e->hasGroup(g)) e->delGroup(g);
		}
	}
public:
	//FIX: Layout -> union perhaps?
	Entity* gridBounds{ nullptr };
	Layout* layout{ nullptr };
	
	UI_Element_Info info;
	
	SpatialComponent* etf = nullptr;
public:
	
	UIComponent(Entity& e) : EComponent(e) { etf = &this->getOwner()->getComponent<SpatialComponent>(); }
	UIComponent(Entity& e, Layout* layout) : EComponent(e), layout(layout) {
		etf = &this->getOwner()->getComponent<SpatialComponent>();
		layout->reallignLayout();
	}
	UIComponent(Entity& e, UIComponent& uic): EComponent(e){}//FIX: implement copy constructor
	Component<Entity>* copyTo(Entity& e) override {
		return &e.addComponent<UIComponent>(*this);
	} 
	~UIComponent() {
		delete layout;
	}

	void addToContainer(Entity& e) {
		if (layout) {
			commonAddToContainerProcesses(&e);
			//if container has set entity limit
			layout->addToLayout(e, e.getComponent<UIComponent>().info);
			layout->reallignLayout();
		}
	}
	std::vector<Entity*> getChildren() { return layout->getContents(); }
	void clear(bool destroyEntities) {
		if(layout) layout->emptyLayout(destroyEntities);
	}

	void init() override {
		if(layout) layout->setBounds(this->getOwner()->getComponent<SpatialComponent>());
	}

	void fixedUpdate(bool EOI) override {
		if(layout) layout->reallignLayout();
	}

	void draw() override {
		//containerBounds is relative to the window
		//entity->draw() will be relative to the in-game world/camera
		//solution: draw() takes a parameter SDL_Rect* indicating which camera to be relative to, if SDL_Rect* == nullptr, just draw to window
		SDL_RenderSetClipRect(Game::renderer, &etf->toRect());
		for (Entity*& e : layout->getContents()) if(e) e->draw();
		SDL_RenderSetClipRect(Game::renderer, nullptr);
	}
	void draw(ViewPort* camera) override {
		//containerBounds is relative to the window
		//entity->draw() will be relative to the in-game world/camera
		//solution: draw() takes a parameter SDL_Rect* indicating which camera to be relative to, if SDL_Rect* == nullptr, just draw to window
		SDL_RenderSetClipRect(Game::renderer, &etf->toRect());
		if(layout) for (Entity*& e : layout->getContents()) if(e) e->draw(camera);
		SDL_RenderSetClipRect(Game::renderer, nullptr);
	}
};

class FreeLayout : public Layout{
	std::vector<Entity*> contents;

	int sizeLimit{ -1 };

	FreeLayout(int sizeLimit) : sizeLimit(sizeLimit), contents(sizeLimit < 0 ? 0 : sizeLimit, nullptr) {}
	
	std::vector<Entity*> getContents() { return contents; }

	bool getNextEmptySlot(Entity*& store) override {
		for (Entity*& slot : contents) {
			//if empty slot found, fill it
			if (slot==nullptr) {
				store = slot;
				return true;
			}
		}
		return false;
	}

	bool addToLayout(Entity& entity, UI_Element_Info& emplaceInfo) override {
		for (Entity*& slot : contents) {
			//if empty slot found, fill it
			if (slot==nullptr) {
				slot = &entity;
				return true;
			}
		}
		//if unable to find empty slot or add new col/row, return as method was unable to add entity to grid
		return false;
	}
};

//FIX: sort out size hinting and sizes.
//		specifically figure out how it should go if size_hint_x: 0.6 is added in row 1 and size_hint_x: 0.3 is added in row 2 in same column. which gets priority?
class GridLayout : public Layout {
private:
	//if cols/rows is -1, it is deemed to be infinite and when a row 
	int cols{ -1 };
	int rows{ -1 };

	std::vector<std::vector<Entity*>> grid;



	void addRow(UI_Element_Info& size) {
		if(grid.size()==0) grid.push_back(std::vector<Entity*>(cols < 0 ? 1 : cols, nullptr));
		else grid.push_back(std::vector<Entity*>(grid.back().size()));
	}
	void addColumn(UI_Element_Info& size) {
		for (std::vector<Entity*>& row : grid) {
			row.push_back(nullptr);
		}
	}

public:
	GridLayout(int cols, int rows) :
		Layout(),
		cols(cols),
		rows(rows),
		grid(rows < 1 ? 1 : rows, std::vector<Entity*>(cols < 1 ? 1 : cols, nullptr)) {
	}
	~GridLayout() {
		for (std::vector<Entity*>& row : grid) {
			for (Entity*& e : row) {
				e->destroy();
			}
		}
	}
	std::vector<Entity*> getContents() { std::vector<Entity*> cs; for (std::vector<Entity*>& row : grid) for (Entity* e : row) cs.push_back(e); return cs; }

	//
	int defaultColWidth{ 100 };
	int defaultRowHeight{ 100 };

	//space between each column / row
	int colSpacing{ 0 };
	int rowSpacing{ 0 };
	//padding from bounds of Grid container;
	PixelPadding padding;


	//override simple add
	bool addToLayout(Entity& entity, UI_Element_Info& emplaceInfo) override {
		// - try to find an empty slot
		// - if reached end of row but unlimited columns, add another column and insert entity into the new slot at the end of the row
		// - if reached end of whole grid, if unlimited rows then add another row and insert entity into 1st slot of that row

		for (std::size_t row = 0; row < grid.size(); ++row) {
			for (std::size_t col = 0; col < grid[row].size(); ++col) {
				//if empty slot found, fill it
				if (grid[row][col] == nullptr) {
					addToLayout(entity, emplaceInfo, col, row);
					return true;
				}
			}
			//if reached end of row and no empty slot -> if infinite columns are allowed, add another column for this entity
			if (cols < 0) {
				addColumn(entity.getComponent<DataComponent<UI_Element_Info>>().data);
				addToLayout(entity, emplaceInfo, grid[row].size()-1, row);
				return true;
			}
		}
		//if reached end of grid and no empty slot -> if infinite rows are allowed, add another row and then fill first slot
		if (rows < 0) {
			addColumn(entity.getComponent<DataComponent<UI_Element_Info>>().data);
			addToLayout(entity, emplaceInfo, 0, grid.size() - 1);
			return true;
		}

		//if unable to find empty slot or add new col/row, return as method was unable to add entity to grid
		return false;
	}
	//new grid index add
	bool addToLayout(Entity& entity, UI_Element_Info& emplaceInfo, std::size_t col, std::size_t row) {
		//if slot out of bounds
		if (row >= grid.size()) return false;
		if (col >= grid[row].size()) return false;

		if (grid[row][col]) grid[row][col]->destroy();
		grid[row][col] = &entity;

		reallignLayout();

	}
	//FIX: make an add to row / column capability so row = 3, col = -1 to indicate row 3 any column

	void emptyLayout(bool d) override {
		for (std::vector<Entity*>& row : grid) {
			for (Entity*& e : row) {
				if (d) e->destroy();;
				e = nullptr;
			}
		}
	}
	void reallignLayout() override {
		//if this layout is not actually in use
		if (gridBounds == nullptr) return;

		SDL_Rect bounds = gridBounds->toRect();
		std::pair<int, int> remainingSize{bounds.w,bounds.h};
		
		//size
		using LargestWidthPointer = int*;
		using LargestHeightPointer = int*;
		//size hint
		using LargestSHWidthPointer = float*;
		using LargestSHHeightPointer =float*;

		std::pair<std::vector<LargestWidthPointer>, std::vector<LargestHeightPointer>> largestStaticSizes{
			std::vector<LargestWidthPointer>(grid[0].size(),nullptr),
			std::vector<LargestHeightPointer>(grid.size(), nullptr) 
		};
		std::pair<std::vector<LargestSHWidthPointer>, std::vector<LargestSHHeightPointer>> largestSizeHints{
			std::vector<LargestSHWidthPointer>(grid[0].size(),nullptr),
			std::vector<LargestSHHeightPointer>(grid.size(), nullptr)
		};

		std::pair<
			std::vector<int*>, //widths
			std::vector<int*>  //heights
		> finalSizes( { 
			std::vector<int*>(grid[0].size(),nullptr),
			std::vector<int*>(grid.size(), nullptr)
			});
		//get data for sizes and size_hints for each row/column
		for (std::size_t row = 0; row < grid.size(); ++row) {
			for (std::size_t col = 0; col < grid[row].size(); ++col) {
				if (grid[row][col] == nullptr) continue;
				std::array<float*, 2> sizeHints = grid[row][col]->getComponent<UIComponent>().info .getSizeHints();
				Size eSize = grid[row][col]->getComponent<UIComponent>().info.size;
				
				//if entity has no size hint x, make note of the entity width if it is larger than the current largest measured width for this column
				//	if has size hint x, do the same but for size hint x
				if (sizeHints[0] != nullptr){
					if (largestSizeHints.first[col] == nullptr || *sizeHints[0] > *largestSizeHints.first[col]) largestSizeHints.first[col] = sizeHints[0];
				}
				else if (largestStaticSizes.first[col] == nullptr || eSize.width > *largestStaticSizes.first[col]) largestStaticSizes.first[col] = &eSize.width;

				
				//repeat for heights
				if (sizeHints[1] != nullptr) {
					if (largestSizeHints.second[row] == nullptr || *sizeHints[1] > *largestSizeHints.second[row]) largestSizeHints.second[row] = sizeHints[1];
				}
				else if (largestStaticSizes.second[row] == nullptr || eSize.height > *largestStaticSizes.second[row]) largestStaticSizes.second[row] = &eSize.height;

			}
			
			//if this row has a static height entity, its height will be the rows height
			if (largestStaticSizes.second[row] != nullptr) {
				finalSizes.second[row] = largestStaticSizes.second[row];
				remainingSize.second -= *largestStaticSizes.second[row];
			}
		}
		//then loop through the columns to
		// A) see which had static size entities
		// B) remove and normalise size hints
		for (std::size_t col = 0; col < grid[0].size(); ++col) {
			if (largestStaticSizes.first[col] != nullptr)
			{
				finalSizes.first[col] = largestStaticSizes.first[col];
				remainingSize.first -= *largestStaticSizes.first[col];
			}
		}

		if (remainingSize.first < 0) {
			gridBounds->width -= remainingSize.first;
			remainingSize.first = 0;
		}
		if (remainingSize.second < 0) {
			gridBounds->height -= remainingSize.second;
			remainingSize.second = 0;
		}

		float tot{ 0.0f };
		for (LargestSHWidthPointer& shx : largestSizeHints.first) {
			if (shx != nullptr) tot += *shx;
		}
		float shxNormaliser = 1.0f / tot;
		
		 tot=  0.0f ;
		for (LargestSHHeightPointer& shy : largestSizeHints.second) {
			if (shy != nullptr) tot += *shy;
		}
		float shyNormaliser = 1.0f / tot;

		//FIX: set x and y positions of each contained entity
		Vector2D pos = gridBounds->position;
		for (std::size_t row = 0; row < grid.size(); ++row) {
			pos.x = gridBounds->position.x;
			float rowHeight;
			if (largestStaticSizes.second[row] != nullptr) {
				rowHeight = *largestStaticSizes.second[row];
			}
			else if (largestSizeHints.second[row] != nullptr) {
				rowHeight = *largestSizeHints.second[row] * shyNormaliser * remainingSize.second;
			}
			else rowHeight = defaultRowHeight;
			for (std::size_t col = 0; col < grid[row].size(); ++col) {
				//FIX: columnWidth and rowHeight can be determined before this loop as the largestSizes have already been recorded
				float columnWidth;

				//set Width
				if (largestStaticSizes.first[col] != nullptr) {
					columnWidth = *largestStaticSizes.first[col];
				}
				else if (largestSizeHints.first[col] != nullptr) {
					columnWidth = *largestSizeHints.first[col] * shxNormaliser * remainingSize.first;
				}
				else columnWidth = defaultColWidth;
				//set height
				
				
				
				if (grid[row][col] != nullptr) {
					SpatialComponent& sc = grid[row][col]->getComponent<SpatialComponent>();
					sc.position.x = pos.x;
					sc.position.y = pos.y;

					sc.width = columnWidth;
					sc.height = rowHeight;
					
				}
				pos.x += columnWidth;
				

			}
			pos.y += rowHeight;
		}
	
	}
};
