#include "../Object.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Pathfinder.h"
#include "../Box2DUtils.h"
#include "../Debug.h"
#include <stdio.h>

void ObjectSkeleton_prePhysics(EventListenerComponent* el) {
	static unsigned stopwatch = 0;
	stopwatch += DeltaTicks();
	if (1 < stopwatch) {
		Object* me = FindObjectOfComponent(el);
		Object* player = FindObjectById(CurrentLevel()->playerId);
		
		List gridSteps;
		ListInit(&gridSteps, sizeof(Vec2I));
		int pathfinderResult = PathfinderMapFindGridSteps(&CurrentLevel()->pathfinderMap, me->position, player->position, &gridSteps);
		//fprintf(stderr, "Pathfinder result: %d\n", pathfinderResult);
		if (pathfinderResult == 0) {
			DebugVec2IList("Grid Path", &gridSteps);

			List anyAngleGridSteps;
			ListInit(&anyAngleGridSteps, sizeof(Vec2I));
			PathfinderMapGridStepsToAnyAngle(&gridSteps, 0.5f, &anyAngleGridSteps);
			DebugVec2IList("Any Angle", &anyAngleGridSteps);

			if (1 < anyAngleGridSteps.bucket.size) {
				uint64_t myPositionIterator = ListGetLast(&anyAngleGridSteps);
				uint64_t targetIterator = ListGetPrev(&anyAngleGridSteps, myPositionIterator);
				Vec2I* targetPosition = ListGetData(&anyAngleGridSteps, targetIterator);
				if (targetPosition) {
					PhysicsComponent* phy = FindPhysicsOfObject(me);
					Vec2F direction = Vec2FSub(Vec2FFromVec2I(*targetPosition), me->position);
					Box2DBodyApplyForceToCenter(phy->body, Vec2FMul(Vec2FNormalize(direction), DeltaTicks() * 12.5f), true);
				}
			}

			ListDeinit(&anyAngleGridSteps);
		}
		ListDeinit(&gridSteps);

		stopwatch -= 1;
	}
}

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
	el->prePhysics = ObjectSkeleton_prePhysics;
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
