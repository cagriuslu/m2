#ifndef COMPONENT_H
#define COMPONENT_H

#include "Vec2F.h"
#include "Box2DWrapper.h"
#include <SDL.h>
#include <stdint.h>

typedef struct _Component {
	uint64_t objId;
} Component;
int ComponentInit(Component* component, uint64_t objectId);
void ComponentDeinit(Component* component);

typedef struct _EventListenerComponent {
	Component super;
	void (*prePhysics)(struct _EventListenerComponent*);
	void (*postPhysics)(struct _EventListenerComponent*);
	void (*preGraphics)(struct _EventListenerComponent*);
	void (*postGraphics)(struct _EventListenerComponent*);
} EventListenerComponent;
int EventListenerComponentInit(EventListenerComponent* evListener, uint64_t objectId);
void EventListenerComponentDeinit(EventListenerComponent* evListener);

typedef struct _PhysicsComponent {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _PhysicsComponent*, struct _PhysicsComponent*);
} PhysicsComponent;
int PhysicsComponentInit(PhysicsComponent* phy, uint64_t objectId);
void PhysicsComponentDeinit(PhysicsComponent* phy);
void PhysicsComponentContactCB(Box2DContact* contact);

typedef struct _GraphicsComponent {
	Component super;
	SDL_Texture* tx;
	SDL_Rect txSrc;
	float txAngle;
	Vec2F txOffset; // w.r.t tx, # of pixels
	void (*draw)(struct _GraphicsComponent*);
} GraphicsComponent;
int GraphicsComponentInit(GraphicsComponent* gfx, uint64_t objectId);
void GraphicsComponentDeinit(GraphicsComponent* gfx);
int GraphicsComponentYComparatorCB(uint64_t gfxIdA, uint64_t gfxIdB);

typedef struct _ComponentDefense {
	Component super;
	int maxHp;
	int hp;
} ComponentDefense;
int ComponentDefenseInit(ComponentDefense* def, uint64_t objId);
void ComponentDefenseDeinit(ComponentDefense* def);

typedef struct _ComponentOffense {
	Component super;
	uint64_t originator; // Object ID
	int ticksLeft;
	int hp;
} ComponentOffense;
int ComponentOffenseInit(ComponentOffense* def, uint64_t objId);
void ComponentOffenseCopyExceptSuper(ComponentOffense* dest, ComponentOffense* src);
void ComponentOffenseDeinit(ComponentOffense* def);

#endif