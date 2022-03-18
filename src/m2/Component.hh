#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/Cfg.hh>
#include <m2/vec2f.hh>
#include <m2/Box2D.hh>
#include <m2/Pool.hh>
#include <SDL.h>
#include <stdint.h>

typedef struct {
	ID objId;
} Component;
M2Err Component_Init(Component* component, ID objectId);
void Component_Term(Component* component);

typedef struct _ComponentMonitor {
	Component super;
	void (*prePhysics)(struct _ComponentMonitor*);
	void (*postPhysics)(struct _ComponentMonitor*);
	void (*preGraphics)(struct _ComponentMonitor*);
	void (*postGraphics)(struct _ComponentMonitor*);
} ComponentMonitor;
M2Err ComponentMonitor_Init(ComponentMonitor* evListener, ID objectId);
void ComponentMonitor_Term(ComponentMonitor* evListener);

typedef struct _ComponentPhysique {
	Component super;
	Box2DBody* body;
	void (*onCollision)(struct _ComponentPhysique*, struct _ComponentPhysique*);
} ComponentPhysique;
M2Err ComponentPhysique_Init(ComponentPhysique* phy, ID objectId);
void ComponentPhysique_Term(ComponentPhysique* phy);
void ComponentPhysique_ContactCB(Box2DContact* contact);

typedef struct _ComponentGraphic {
	Component super;
	SDL_Texture *texture;
	SDL_Rect textureRect;
	m2::vec2f center_px;
	float angle;
	void (*draw)(struct _ComponentGraphic*);
} ComponentGraphic;
M2Err ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId);
void ComponentGraphic_Term(ComponentGraphic* gfx);
void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx);
void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio);
int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB);

typedef struct _ComponentLight {
	Component super;
	float radius_m;
	void (*draw)(struct _ComponentLight*);
} ComponentLight;
M2Err ComponentLight_Init(ComponentLight* lig, ID objectId);
void ComponentLight_Term(ComponentLight* lig);
void ComponentLight_DefaultDraw(ComponentLight* lig);

typedef struct _ComponentDefense {
	Component super;
	char data[];
} ComponentDefense;
M2Err ComponentDefense_Init(ComponentDefense* def, ID objId);
void ComponentDefense_Term(ComponentDefense* def);

typedef struct {
	Component super;
	char data[];
} ComponentOffense;
M2Err ComponentOffense_Init(ComponentOffense* def, ID objId);
void ComponentOffense_Term(ComponentOffense* def);

#endif
