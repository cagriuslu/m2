#include <m2/Component.h>
#include <m2/Object.h>
#include <m2/Game.h>

m2::Component::Component(Id object_id) : object_id(object_id) {}

m2::Object& m2::Component::parent() const {
    return *M2_LEVEL.objects.get(object_id);
}

std::function<bool(const m2::Component&)> m2::is_component_of_child_object_of_parent(Id parent_id) {
	return [parent_id](const Component& c) { return c.parent().parent_id() == parent_id; };
}
