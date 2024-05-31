#pragma once

#include <m2/component/Graphic.h>
#include <m2/component/Character.h>

// Draws resources to the bottom-right corner of the Factory.
// Only one type of resource is drawn. If the Factory contains more than one type resource, exception is thrown.
void draw_resources(m2::Character& chr);
