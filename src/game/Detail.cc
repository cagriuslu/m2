#include <m2/game/Detail.h>
#include <m2/Meta.h>

m2::VecF m2::tile_position_f(const VecI& v) {
	return {F(v.x) + 0.5f, F(v.y) + 0.5f};
}

m2::VecI m2::tile_position_i(const VecF& v) {
	return VecI{v};
}
