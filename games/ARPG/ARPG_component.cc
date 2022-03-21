#include <game/component.hh>

game::component_defense::component_defense(ID object_id) : m2::component_defense(object_id), maxHp(0), hp(0), onHit(nullptr), onDeath(nullptr) {

}

game::component_offense::component_offense(ID object_id) : m2::component_offense(object_id), originator(0) {

}
