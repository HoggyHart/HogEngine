#pragma once
#include <string>
#include <vector>
#include <functional>
#include <queue>
#include <SDL.h>
typedef unsigned int Uint32;
//CONTINUE: still working on concept, component makeup to be done later
/*******/
//FIX/CONTINUE: not "fix" necessarily BUT:
//		InputController listens for input events,
//		GameEventListener listens for game events
//		Log listens for whatever its wants to. still needs to be sorted
//	PERHAPS they could all inherit from the same class? they are similar in that they listen so maybe at least for the sake of drawing a solid link between them they could inherit from "Subscriber" or something.
//	 then maybe listener could include some of the base functionalities of them? idk
//

enum TopicType { topicNone, topicPublisher, topicListener };

struct Topic {
private:
	bool active{ true };
public:
	TopicType type{ topicNone };
	std::string topicID;

	Topic(TopicType type, std::string id);
	virtual ~Topic() {};
	virtual void fixedUpdate() {};
	void deactivate() { active = false; }
	bool isActive() { return active; }
};

template<typename T>
struct Publisher : public Topic {
private:
	Uint32 lastPublish{ 0 };
public:
	//method should capture all relevant info when created
	std::function<T* (Publisher<T>&)> publishingMethod;
	T* newMessage{ nullptr };
	Uint32 publishDelay;

	Publisher(std::string name, Uint32 publishDelay, std::function<T* (Publisher<T>&)> publishMethod) : Topic(topicPublisher, name), publishDelay(publishDelay), publishingMethod(publishMethod) {}
	~Publisher() {
		delete newMessage;
	}
	void fixedUpdate() override {
		if (newMessage != nullptr) {
			delete newMessage;
			newMessage = nullptr;
		}

		if (SDL_GetTicks() - lastPublish >= publishDelay) {
			newMessage = publishingMethod(*this);
			lastPublish = SDL_GetTicks();
		}
	}
};

template<typename T>
struct Subscriber : public Topic {
	std::function<void(T&)> processingMethod{};

	Subscriber(std::string listenedTopic, std::function<void(T&)> process) : Topic(topicListener, listenedTopic), processingMethod(process) {}

	void fixedUpdate() override;
};

template<typename T>
struct Logger : public Subscriber<T> {
	std::vector<T> messages;

	Logger(std::string topic, Uint32 logSize) : Subscriber<T>(topic,
		[this, logSize](T& msg) {
			if (this->messages.size() >= logSize) this->messages.erase(this->messages.begin());
			this->messages.push_back(msg);
		})
	{
	}
};

struct Node {
	Topic* content{ nullptr };

	Node(Topic& t) : content(&t) {};

	void fixedUpdate();
};

class Nodes {
	static std::vector<Node> nodes;
public:
	static void addNode(Topic& t) {
		nodes.push_back(Node{ t });
	}
	static void updateNodes() {
		for (Node& n : nodes) {
			n.fixedUpdate();
		}
	}

	template<typename T>
	static Publisher<T>* getPublisher(std::string topicName);
};

//add node deactivation stuff. if a publisher is created, and the thing it is tracking is destroyed, destroy the publisher too
template<typename T>
inline Publisher<T>* Nodes::getPublisher(std::string topicName)
{
	//look through nodes for node containing topic with type publisher that has this name;
	for (Node& n : nodes) {
		if (n.content != nullptr && n.content->topicID == topicName && n.content->type == topicPublisher) return static_cast<Publisher<T>*>(n.content);
	}
	return nullptr;
}

template<typename T>
inline void Subscriber<T>::fixedUpdate()
{
	Publisher<T>* pub = Nodes::getPublisher<T>(topicID);
	if (pub != nullptr && pub->newMessage != nullptr) {
		processingMethod(*pub->newMessage);
	}
}