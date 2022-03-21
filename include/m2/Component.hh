#ifndef COMPONENT_H
#define COMPONENT_H

#include <m2/Cfg.hh>
#include <m2/Vec2F.h>
#include <m2/Box2D.hh>
#include <b2_body.h>
#include <SDL.h>
#include <stdint.h>

struct Component {
	ID objId;
};
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
	b2Body* body;
	void (*onCollision)(struct _ComponentPhysique*, struct _ComponentPhysique*);
} ComponentPhysique;
M2Err ComponentPhysique_Init(ComponentPhysique* phy, ID objectId);
void ComponentPhysique_Term(ComponentPhysique* phy);
void ComponentPhysique_ContactCB(b2Contact* contact);

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

namespace m2 {
    struct component_defense {
        Component super;

        component_defense() = default;
        explicit component_defense(ID obj_id);
    };

    struct component_offense {
        Component super;

        component_offense() = default;
        explicit component_offense(ID obj_id);
    };
}

#endif
