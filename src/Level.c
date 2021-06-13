#include "Level.h"
#include "EventListenerComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"
#include "Box2DWrapper.h"
#include "Levels/LevelCommon.h"

// TODO move this to GraphicsComponent
static int graphicsYComparator(void* graphicsA, void* graphicsB) {
	// TODO
}

int LevelInit(Level* level) {
	memset(level, 0, sizeof(Level));
	PROPAGATE_ERROR(BucketInit(&level->objects, sizeof(Object)));
	PROPAGATE_ERROR(InsertionListInit(&level->drawList, sizeof(uint32_t), UINT16_MAX + 1, graphicsYComparator));
	PROPAGATE_ERROR(BucketInit(&level->eventListeners, sizeof(EventListenerComponent)));
	PROPAGATE_ERROR(BucketInit(&level->physics, sizeof(PhysicsComponent)));
	PROPAGATE_ERROR(BucketInit(&level->graphics, sizeof(GraphicsComponent)));
	level->world = Box2DWorldCreate((Vec2F) { 0.0f, 0.0f });
	level->contactListener = Box2DContactListenerRegister(PhysicsComponentContactCB);
	Box2DWorldSetContactListener(level->world, level->contactListener);
}

void LevelDeinit(Level* level) {
	Box2DContactListenerDestroy(level->contactListener);
	Box2DWorldDestroy(level->world);
	BucketDeinit(&level->graphics);
	BucketDeinit(&level->physics);
	BucketDeinit(&level->eventListeners);
	InsertionListDeinit(&level->drawList);
	BucketDeinit(&level->objects);
	memset(level, 0, sizeof(Level));
}

int LevelLoadTest(Level* level) {

}

int LevelLoadEditor(Level* level) {

}







void LevelUnload() {
	LevelCallUnloader();
}
