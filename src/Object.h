#ifndef OBJECT_H
#define OBJECT_H

#include "Point.h"
#include "Geometry.h"

typedef struct _Object {
	Point pos; // Position of the origin of the object
	// Physics subsystem
	Geometry geo; // Geometry
	Point vel; // Velocity
	Point avel; // Angular velocity
	Point drag; // Drag
	Point adrag; // Angular drag
	float mass; // Mass
	Point grav; // Effecting gravitational force
	// Trigger subsystem
	Geometry trig; // Geometry
} Object;

#endif
