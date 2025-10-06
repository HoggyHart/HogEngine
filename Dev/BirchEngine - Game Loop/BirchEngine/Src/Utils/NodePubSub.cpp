#include "Src/Utils/NodePubSub.hpp"
std::vector<Node> Nodes::nodes;

void Node::fixedUpdate()
{
	if (content) {
		if (content->isActive()) content->fixedUpdate();
		else {
			delete content;
			content = nullptr;
		}
	}
}

Topic::Topic(TopicType type, std::string id) : type(type), topicID(id) {
	Nodes::addNode(*this);
}