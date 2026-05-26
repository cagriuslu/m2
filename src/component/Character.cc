#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>

using namespace m2;

bool m2::IsDescendantOf(ObjectId objId, const ObjectId parentId) {
	while (objId) {
		if (objId == parentId) { return true; }
		objId = M2_LEVEL.objects[objId].GetParentId();
	}
	return false;
}

m2g::pb::CardCategory m2::ToCategoryOfCard(const m2g::pb::CardType ct) {
	return M2_GAME.GetCard(ct).Category();
}
