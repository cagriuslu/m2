#include "impl/public/Component.hh"

impl::Defense::Defense(ID object_id) : m2::component::Defense(object_id), maxHp(0), hp(0), onHit(nullptr), onDeath(nullptr) {

}

impl::Offense::Offense(ID object_id) : m2::component::Offense(object_id), originator(0) {

}
