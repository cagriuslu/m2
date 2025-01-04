#include <mine/object/Blacksmith.h>
#include <m2/Game.h>

using namespace m2g;
using namespace m2g::pb;

m2::void_expected create_blacksmith(m2::Object& obj) {
	obj.add_graphic(BLACKSMITH_FULL);

	return {};
}
