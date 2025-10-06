#include "SpatialComponent.hpp"
#include "LinkComponent.hpp"
LinkComponent& SpatialComponent::addLink(SpatialComponent& tc, std::pair<Vector2D, Vector2D> offsets) {
	Entity& e = this->getOwner()->entityManager.queueAddEntity<Entity>();
	LinkComponent& link = e.addComponent<LinkComponent>(this, &tc);
	link.setOffsets(offsets.first, offsets.second);

	this->links.push_back(&link);
	tc.links.push_back(&link);

	return link;
}

LinkComponent& SpatialComponent::addLink(SpatialComponent& tc) {
	std::pair<Vector2D, Vector2D> offsets{ Vector2D{this->width,this->height}*this->scale / 2.0f, Vector2D{tc.width,tc.height}*tc.scale / 2.0f };
	return this->addLink(tc, offsets);
}

void SpatialComponent::closeLink(LinkComponent* link) {
	//THERE IS PROBABLY A BETTER WAY TO DO THIS
	/*if (link->tc1 == this) {
		this->linksOut.erase(std::remove_if(this->linksOut.begin(), this->linksOut.end(), [link](LinkComponent*& b) {return link == b; }), this->linksOut.end());
		link->tc2->linksIn.erase(std::remove_if(link->tc2->linksIn.begin(), link->tc2->linksIn.end(), [link](LinkComponent*& b) {return link == b; }), link->tc2->linksIn.end());
	}
	else {
		this->linksIn.erase(std::remove_if(this->linksIn.begin(), this->linksIn.end(), [link](LinkComponent*& b) {return link == b; }), this->linksIn.end());
		link->tc1->linksOut.erase(std::remove_if(link->tc1->linksOut.begin(), link->tc1->linksOut.end(), [link](LinkComponent*& b) {return link == b; }), link->tc1->linksOut.end());
	}*/
	link->unbindLinkPoints();
}

void SpatialComponent::closeAllLinks() {
	//could merge linksIn and linksOut into one toDelete vector, but in edge case where a link has been made between a TC and itself,
	//std::vector<LinkComponent*> toDelete = linksIn;
	//for (LinkComponent*& l : toDelete) {
	//	closeLink(l);
	//	//delete l;
	//}
	//toDelete = linksOut;
	//for (LinkComponent*& l : toDelete) {
	//	closeLink(l);
	//	//delete l;
	//}
	std::vector<LinkComponent*> toDelete = links;
	for (LinkComponent*& l : toDelete) {
		l->unbindLinkPoints();
	}
}