#ifndef IMPL_DROP_H
#define IMPL_DROP_H

#include <impl/private/item/Item.h>
#include <m2/Object.h>
#include <m2/Def.h>

namespace impl::object {
	struct Drop : public m2::ObjectImpl {
		const item::ItemBlueprint& item_blueprint;

		explicit Drop(const item::ItemBlueprint& item_blueprint);

		static M2Err init(m2::Object& obj, const item::ItemBlueprint& item_blueprint, m2::Vec2f pos);
	};
}

#endif //IMPL_DROP_H
