#include <pinball/objects/Player.h>
#include <m2/Game.h>
#include <pinball/Pinball.h>
#include <pinball/objects/Edge.h>

m2::void_expected LoadPlayer(m2::Object& obj) {
	M2_LEVEL.player_id = obj.id();
	obj.position = gLevelCenter;
	M2_GAME.SetGameHeightM(gLevelDimensions.y);

	LoadEdge();

	return {};
}
