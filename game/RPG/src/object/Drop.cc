#include <rpg/object/Drop.h>

obj::Drop::Drop(const itm::ItemBlueprint &item_blueprint) : item_blueprint(item_blueprint) {}

M2Err obj::Drop::init(m2::Object &obj, const itm::ItemBlueprint &item_blueprint, m2::Vec2f pos) {
	// TODO
	return M2OK;
}
