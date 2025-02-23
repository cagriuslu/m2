#include <m2/Component.h>
#include <m2/Object.h>
#include <m2/Game.h>

m2::Component::Component(Id owner_id) : _owner_id(owner_id) {}

m2::Object& m2::Component::owner() const {
    return *M2_LEVEL.objects.Get(_owner_id);
}

std::function<bool(const m2::Component&)> m2::is_component_of_parent_object(Id parent_id) {
	return [parent_id](const Component& c) {
		auto obj_id = c.owner_id();
		while (obj_id) {
			if (obj_id == parent_id) {
				return true;
			}
			obj_id = M2_LEVEL.objects[obj_id].parent_id();
		}
		return false;
	};
}
