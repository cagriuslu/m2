#include <mine/object/Dwarf.h>
#include <m2/Game.hh>
#include <SpriteType.pb.h>

m2::VoidValue create_dwarf(m2::Object& obj) {
	obj.add_graphic(GAME.sprites[m2g::pb::SpriteType::DWARF_FULL]);

	GAME.playerId = obj.id();
	return {};
}
