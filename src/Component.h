#ifndef COMPONENT_H
#define COMPONENT_H

#include "Vec2F.h"
#include "Box2DWrapper.h"
#include <SDL.h>
#include <stdint.h>

typedef struct _Component {
	uint32_t objId;
} Component;
int ComponentInit(Component* component, uint32_t objectId);
void ComponentDeinit(Component* component);

typedef struct _EventListenerComponent {
	Component super;
	void (*prePhysics)(struct _EventListenerComponent*);
	void (*postPhysics)(struct _EventListenerComponent*);
	void (*preGraphics)(struct _EventListenerComponent*);
	void (*postGraphics)(struct _EventListenerComponent*);
} EventListenerComponent;
int EventListenerComponentInit(EventListenerComponent* evListener, uint32_t objectId);
void EventListenerComponentDeinit(EventListenerComponent* evListener);

typedef struct _PhysicsComponent {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _PhysicsComponent*, struct _PhysicsComponent*);
} PhysicsComponent;
int PhysicsComponentInit(PhysicsComponent* phy, uint32_t objectId);
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
int GraphicsComponentInit(GraphicsComponent* gfx, uint32_t objectId);
void GraphicsComponentDeinit(GraphicsComponent* gfx);
int GraphicsComponentYComparatorCB(uint32_t gfxIdA, uint32_t gfxIdB);

typedef struct _ComponentDefense {
	Component super;
	int maxHp;
	int hp;
} ComponentDefense;
int ComponentDefenseInit(ComponentDefense* def, uint32_t objId);
void ComponentDefenseDeinit(ComponentDefense* def);

typedef struct _ComponentOffense {
	Component super;
	uint32_t originator; // Object ID
	int hp;
} ComponentOffense;
int ComponentOffenseInit(ComponentOffense* def, uint32_t objId);
void ComponentOffenseCopyExceptSuper(ComponentOffense* dest, ComponentOffense* src);
void ComponentOffenseDeinit(ComponentOffense* def);

#endif
