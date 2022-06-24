#include <m2g/component/Defense.h>

m2g::comp::Defense::Defense(m2::ID object_id) : m2::comp::Defense(object_id), maxHp(0), hp(0), on_hit(nullptr), on_death(nullptr) {}
