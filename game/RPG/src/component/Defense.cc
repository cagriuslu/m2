#include <m2g/component/Defense.h>

m2g::component::Defense::Defense(ID object_id) : m2::component::Defense(object_id), maxHp(0), hp(0), on_hit(nullptr), on_death(nullptr) {}
