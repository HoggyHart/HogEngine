#pragma once

//Base
#include "Src/ECS/ECS.hpp"

//STANDALONE COMPONENTS
#include "DurationComponent.hpp"
#include "SpatialComponent.hpp"
#include "ResourcesComponent.hpp"
#include "CustomDataStructureComponent.hpp"
#include "CustomFuncComponent.hpp"
//dependancy included components
#include "MovementComponent.hpp"
#include "SpriteComponent.hpp"
#include "TextComponent.hpp"
#include "ColliderComponent.hpp"
#include "InputController.hpp" // FIX: Arcs a bit too complex, could replace with some use of circle and square cross section? i.e. circle hitbox, then 2 rect hitboxes alligned with the edges of the arc.
#include "ProjectileComponent.hpp"
#include "TileComponent.hpp" //NOTE: Basically a SpriteComponent with fewer parts, good for optimisation i think, but some tiles may be animated, so we'll see (i.e. waterfall tile?)
#include "TrackerComponent.hpp" //FIX : I believe this could be put in as a variant of CustomDataStructure component?
#include "AIComponent.hpp"
#include "LineComponent.hpp"
#include "ButtonComponent.hpp"
//#include "ContainerComponent.hpp"
#include "FadeAwayComponent.hpp"
#include "TrailComponent.hpp"
#include "InteractableComponent.hpp"
#include "LinkComponent.hpp"
#include "GameEventListener.hpp"
#include "UIComponent.hpp"