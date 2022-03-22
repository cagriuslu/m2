#include <m2/Component.hh>

Monitor::Monitor(ID object_id) : Component(object_id), prePhysics(nullptr), postPhysics(nullptr), preGraphics(nullptr), postGraphics(nullptr) {}
