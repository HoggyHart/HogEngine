#include "Src/ECS/components.hpp"

////CONTINUE: still working on concept, component makeup to be done later
//struct Message {
//	Uint32 timestamp;
//	std::string content;
//};
////FIX: not "fix" necessarily BUT:
////		InputController listens for input events,
////		GameEventListener listens for game events
////		Log listens for whatever its wants to. still needs to be sorted
////	PERHAPS they could all inherit from the same class? they are similar in that they listen so maybe at least for the sake of drawing a solid link between them they could inherit from "Subscriber" or something.
////	 then maybe listener could include some of the base functionalities of them
////
//struct Log {
//	std::string topicSubscribedTo;
//	std::vector<Message> messages;
//};
//
//typedef std::vector<Message> Log;
////CHATLOG ONLY STORES AND DISPLAYS MESSAGE. DIFFERENT ENTITY HANDLES ADDING MESSAGES
//class LogBox : public Entity {
//
//	LogBox(Manager& man,int x, int y, int w, int h) : Entity(man) {
//		this->addComponent<DataComponent<Log>>();
//		SpatialComponent& sc = this->addComponent<SpatialComponent>(x, y, w, h, 1.0f);
//		this->addComponent<UIComponent>(GridLayout{ 1,10 });
//		this->addComponent<DrawComponent>(sc, SDL_Color{0,0,0,255});
//	}
//	void logMessage(std::string msg) {
//		UIComponent& cc = this->getComponent<UIComponent>();
//
//	}
//};