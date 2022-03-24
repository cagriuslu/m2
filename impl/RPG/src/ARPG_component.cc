#include <impl/component.hh>

impl::Defense::Defense(ID object_id) : m2::component_defense(object_id), maxHp(0), hp(0), onHit(nullptr), onDeath(nullptr) {

}

impl::Offense::Offense(ID object_id) : m2::component_offense(object_id), originator(0) {

}
