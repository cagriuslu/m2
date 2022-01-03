#ifndef COMPONENT_H
#define COMPONENT_H

#include "Vec2F.h"
#include "Box2D.h"
#include "Pool.h"
#include <SDL.h>
#include <stdint.h>

typedef struct _Component {
	ID objId;
} Component;
int Component_Init(Component* component, ID objectId);
void Component_Term(Component* component);

typedef struct _ComponentEventListener {
	Component super;
	void (*prePhysics)(struct _ComponentEventListener*);
	void (*postPhysics)(struct _ComponentEventListener*);
	void (*preGraphics)(struct _ComponentEventListener*);
	void (*postGraphics)(struct _ComponentEventListener*);
} ComponentEventListener;
int EventListenerComponent_Init(ComponentEventListener* evListener, ID objectId);
void EventListenerComponent_Term(ComponentEventListener* evListener);

typedef struct _ComponentPhysics {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _ComponentPhysics*, struct _ComponentPhysics*);
} ComponentPhysics;
int PhysicsComponent_Init(ComponentPhysics* phy, ID objectId);
void PhysicsComponent_Term(ComponentPhysics* phy);
void PhysicsComponent_ContactCB(Box2DContact* contact);

typedef struct _ComponentGraphics {
	Component super;
	SDL_Rect textureRect;
	Vec2F center_px;
	float angle;
	void (*draw)(struct _ComponentGraphics*);
} ComponentGraphics;
int GraphicsComponent_Init(ComponentGraphics* gfx, ID objectId);
void GraphicsComponent_Term(ComponentGraphics* gfx);
void GraphicsComponent_DefaultDraw(ComponentGraphics* gfx);
void GraphicsComponent_DefaultDrawHealthBar(ComponentGraphics* gfx, float healthRatio);
int GraphicsComponent_YComparatorCB(ID gfxIdA, ID gfxIdB);

typedef struct _ComponentDefense {
	Component super;
	float maxHp;
	float hp;
} ComponentDefense;
int ComponentDefense_Init(ComponentDefense* def, ID objId);
void ComponentDefense_Term(ComponentDefense* def);

typedef struct _ComponentOffense {
	Component super;
	ID originator;
	float ttl;
	float hp;
} ComponentOffense;
int ComponentOffense_Init(ComponentOffense* def, ID objId);
void ComponentOffense_Term(ComponentOffense* def);

#endif
