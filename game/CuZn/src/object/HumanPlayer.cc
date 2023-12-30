#include <cuzn/object/HumanPlayer.h>
#include <m2/Level.h>

m2::void_expected cuzn::init_human_player(m2::Object& obj) {
	auto& chr = obj.add_full_character();
	chr.add_resource(m2g::pb::MONEY, 17.0f);

	auto [income_marker, _] = m2::create_object({}, m2g::pb::INCOME_LEVEL_MARKER, obj.id());
	income_marker.add_tiny_character();

	return {};
}
