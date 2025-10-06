#include "AssetManager.hpp"
#include "Src/Gameplay/ItemStuff/Item.hpp"
#include "Src/Game Management/Scene.hpp"

StaticGameDataManager::StaticGameDataManager() {
	this->init(); 
}

void StaticGameDataManager::init() {
	
	this->initImages();
	this->initFonts();
	//this->initAudio();
	this->initActions();
	this->initItems();
	this->initScenes();
	this->initBehaviours();
}

Scene* StaticGameDataManager::getScene(std::string sn)
{
	return scenes->getObject(sn);
}

SDL_Texture* StaticGameDataManager::getTexture(std::string texID)
{
	return textures->getObject(texID);
}

TTF_Font* StaticGameDataManager::getFont(std::string font)
{
	return fonts->getObject(font);
}

void StaticGameDataManager::initImages() {
	textures = new ImageManager();
	
	//sprites
	textures->addObject("greenGuy", TextureManager::LoadTexture("assets/Images/SpriteSheets/greenGuy.png"));
	textures->addObject("slime", TextureManager::LoadTexture("assets/Images/SpriteSheets/bounceSlime.png"));
	//textures
	textures->addObject("MT", TextureManager::LoadTexture("assets/Images/Textures/missingTexture.png"));
	textures->addObject("itemSlot", TextureManager::LoadTexture("assets/Images/Textures/InventorySlot.png"));
	textures->addObject("cmplx", TextureManager::LoadTexture("assets/Images/Textures/complexImg.png"));
	textures->addObject("mapTiles", TextureManager::LoadTexture("assets/Images/Textures/TileMapSpriteSheet.png"));
	textures->addObject("hitbox", TextureManager::LoadTexture("assets/Images/Textures/hitbox1.png"));
	textures->addObject("bulletPNG", TextureManager::LoadTexture("assets/Images/Textures/bullet.png"));
	textures->addObject("letterC", TextureManager::LoadTexture("assets/Images/Textures/letterC.png"));
	textures->addObject("fastaPasta", TextureManager::LoadTexture("assets/Images/Textures/fastaPasta.png"));
	textures->addObject("growthMix", TextureManager::LoadTexture("assets/Images/Textures/growthMix.png"));
	textures->addObject("baseballBat", TextureManager::LoadTexture("assets/Images/Textures/baseballbat.png"));
	textures->addObject("skibidiToilet", TextureManager::LoadTexture("assets/Images/Textures/skibiditoilet.png"));
}

void StaticGameDataManager::initFonts()
{
	fonts = new FontManager();

	fonts->addObject("Consolasx24", TTF_OpenFont("assets/Fonts/Consolas.ttf", 24));
}

void StaticGameDataManager::initActions()
{
	
}

void StaticGameDataManager::initItems()
{
	items = new ItemManager();
}

void StaticGameDataManager::initBehaviours()
{
	
}

void StaticGameDataManager::initScenes()
{
	scenes = new SceneManager();

	scenes->addObject("MainMenu", new Scene("scenes/mainmenu/"));
	scenes->addObject("DungeonEntrance", new Scene("scenes/startzone/"));
	scenes->addObject("Debug", new Scene("scenes/debugArea/"));
}

