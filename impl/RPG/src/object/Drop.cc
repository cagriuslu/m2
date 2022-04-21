#include <impl/private/object/Drop.h>

impl::object::Drop::Drop(const item::ItemBlueprint &item_blueprint) : item_blueprint(item_blueprint) {}

M2Err impl::object::Drop::init(m2::Object &obj, const item::ItemBlueprint &item_blueprint, m2::Vec2f pos) {
	// TODO
	return M2OK;
}
