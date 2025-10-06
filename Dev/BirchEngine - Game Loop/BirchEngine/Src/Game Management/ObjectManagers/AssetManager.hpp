#pragma once
#include "Src/Game Management/ObjectManagement.hpp"

#include <SDL.h>
#include <SDL_ttf.h>
class Scene;
class Item;
typedef ObjectManager<TTF_Font> FontManager;
typedef ObjectManager<SDL_Texture> ImageManager;
typedef ObjectManager<Scene> SceneManager;
typedef ObjectManager<Item> ItemManager;


class StaticGameDataManager{
public:
	StaticGameDataManager();
	void init();
	Scene* getScene(std::string sn);
	SDL_Texture* getTexture(std::string texID);
	TTF_Font* getFont(std::string font);
private:
	ObjectManager<SDL_Texture>* textures{ nullptr };
	ObjectManager<TTF_Font>* fonts{ nullptr };
	ObjectManager<Scene>* scenes{ nullptr };
	ObjectManager<Item>* items{ nullptr };

	void initImages();
	void initFonts();
	void initActions();
	void initItems();
	void initBehaviours();
	void initScenes();
};