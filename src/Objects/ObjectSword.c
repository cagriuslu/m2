#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include "../Box2DUtils.h"

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

void Sword_prePhysics(EventListenerComponent* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics) {
		PhysicsComponent* phy = FindPhysicsOfObject(obj);
		if (phy && phy->body) {
			Box2DBodySetAngularVelocity(phy->body, -15.0f);
		}
	}
}

void Sword_postPhysics(EventListenerComponent* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics && obj->graphics && obj->offense) {
		PhysicsComponent* phy = FindPhysicsOfObject(obj);
		GraphicsComponent* gfx = FindGraphicsOfObject(obj);
		ComponentOffense* off = FindOffenseOfObject(obj);
		if (phy && phy->body && gfx && off && off->originator) {
			Object* originator = BucketGetById(&CurrentLevel()->objects, off->originator);
			if (originator) {
				Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
			}
			gfx->txAngle = Box2DBodyGetAngle(phy->body);
		}
		
	}
}

int ObjectSwordInit(Object* obj, Vec2F position, float fromAngleDegrees, float toAngleDegrees, ComponentOffense** outOffense) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->prePhysics = Sword_prePhysics;
	el->postPhysics = Sword_postPhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateBody(
		phyId,
		false, // isDisk
		true, // isDynamic
		position,
		false, // allowSleep
		false, // isBullet
		true, // isSensor
		PLAYER_MELEE_WEAPON_CATEGORY, // category
		0, // mask
		(Vec2F) {1.25f, 0.1667f}, // boxDims
		(Vec2F) {0.5833f, 0.0f}, // boxCenterOffset
		0.0f, // boxAngle
		NAN, // diskRadius
		1.0f, // mass
		0.0f, // linearDamping
		false // fixedRotation
	);

	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txAngle = 0.0f;
	gfx->txSrc = (SDL_Rect){ 6 * TILE_WIDTH, 4 * TILE_WIDTH, 2 * TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ -14.0f, 0.0f };

	ComponentOffense* off = ObjectAddAndInitOffense(obj, NULL);
	if (outOffense) {
		*outOffense = off;
	}
	// TODO do not return to be copied after, Receive it so that you can copy here
	
	return 0;
}
