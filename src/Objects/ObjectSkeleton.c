#include "../Object.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"

void ObjectSkeleton_postPhysics(EventListenerComponent *el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj) {
		ComponentDefense* defense = FindDefenseOfObject(obj);
		if (defense && defense->hp <= 0) {
			ArrayAppend(&CurrentLevel()->deleteList, &el->super.objId);
		}
	}
}

int ObjectSkeletonInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->postGraphics = ObjectSkeleton_postPhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateDynamicDisk(
		phyId,
		position,
		ALLOW_SLEEP,
		ENEMY_CATEGORY,
		0.2083f, // Radius
		10.0f, // Mass
		10.0f // Damping
	);

	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txSrc = (SDL_Rect){2 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	gfx->txOffset = (Vec2F){ 0.0f, -4.5f };

	ComponentDefense* defense = ObjectAddAndInitDefense(obj, NULL);
	defense->hp = 100;
	defense->maxHp = 100;
	
	return 0;
}
