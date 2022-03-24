#include <m2/component/Monitor.h>

m2::component::Monitor::Monitor(ID object_id) : Component(object_id), prePhysics(nullptr), postPhysics(nullptr), preGraphics(nullptr), postGraphics(nullptr) {}
