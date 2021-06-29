#ifndef COMPONENT_H
#define COMPONENT_H

#include "Vec2F.h"
#include "Box2D.h"
#include "Bucket.h"
#include <SDL.h>
#include <stdint.h>

typedef struct _Component {
	ID objId;
} Component;
int ComponentInit(Component* component, ID objectId);
void ComponentDeinit(Component* component);

typedef struct _EventListenerComponent {
	Component super;
	void (*prePhysics)(struct _EventListenerComponent*);
	void (*postPhysics)(struct _EventListenerComponent*);
	void (*preGraphics)(struct _EventListenerComponent*);
	void (*postGraphics)(struct _EventListenerComponent*);
} EventListenerComponent;
int EventListenerComponentInit(EventListenerComponent* evListener, ID objectId);
void EventListenerComponentDeinit(EventListenerComponent* evListener);

typedef struct _PhysicsComponent {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _PhysicsComponent*, struct _PhysicsComponent*);
} PhysicsComponent;
int PhysicsComponentInit(PhysicsComponent* phy, ID objectId);
void PhysicsComponentDeinit(PhysicsComponent* phy);
void PhysicsComponentContactCB(Box2DContact* contact);

typedef struct _GraphicsComponent {
	Component super;
	SDL_Texture* tx;
	SDL_Rect txSrc;
	float txAngle;
	Vec2F txCenter; // w.r.t. texture center in pixels, offsets the texture and the center for rotation
	void (*draw)(struct _GraphicsComponent*);
} GraphicsComponent;
int GraphicsComponentInit(GraphicsComponent* gfx, ID objectId);
void GraphicsComponentDeinit(GraphicsComponent* gfx);
int GraphicsComponentYComparatorCB(ID gfxIdA, ID gfxIdB);

typedef struct _ComponentLightSource {
	Component super;
	ID spatialIterator;
	float power;
	Vec2F offset;
	Vec2F direction;
} ComponentLightSource;
int ComponentLightSourceInit(ComponentLightSource* light, ID objectId, float lightBoundaryRadius);
void ComponentLightSourceDeinit(ComponentLightSource* light);
void ComponentLightSourceUpdatePosition(ComponentLightSource* light);

typedef struct _ComponentDefense {
	Component super;
	int maxHp;
	int hp;
} ComponentDefense;
int ComponentDefenseInit(ComponentDefense* def, ID objId);
void ComponentDefenseDeinit(ComponentDefense* def);

typedef struct _ComponentOffense {
	Component super;
	ID originator; // Object ID
	int ticksLeft;
	int hp;
} ComponentOffense;
int ComponentOffenseInit(ComponentOffense* def, ID objId);
void ComponentOffenseCopyExceptSuper(ComponentOffense* dest, ComponentOffense* src);
void ComponentOffenseDeinit(ComponentOffense* def);

#endif
