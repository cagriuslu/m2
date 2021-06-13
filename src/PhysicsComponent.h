#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include "Component.h"
#include "Box2DWrapper.h"

typedef struct _PhysicsComponent {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _PhysicsComponent*, struct _PhysicsComponent*);
} PhysicsComponent;

int PhysicsComponentInit(PhysicsComponent* phy, uint32_t objectId);
void PhysicsComponentDeinit(PhysicsComponent* phy);

void PhysicsComponentContactCB(Box2DContact* contact);

#endif
