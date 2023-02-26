#include <m2/Component.h>
#include <m2/Object.h>
#include <m2/Game.h>

m2::Component::Component(Id object_id) : object_id(object_id) {}

m2::Object& m2::Component::parent() const {
    return *LEVEL.objects.get(object_id);
}
