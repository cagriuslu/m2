#include <m2g/component/Defense.h>

m2g::Defense::Defense(m2::Id object_id) : m2::Defense(object_id), maxHp(0), hp(0), on_hit(nullptr), on_death(nullptr) {}
