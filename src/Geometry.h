#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "Vec2F.h"

typedef enum _GeometryType {
	GEOMETRY_TYPE_NONE = 0,
	GEOMETRY_TYPE_RECTANGLE,
	GEOMETRY_TYPE_CIRCLE
} GeometryType;

typedef struct _GeometryTypeRectangle {
	Vec2F off;
	float w, h;
	float rotZ; // Rotation around Z axis
} GeometryTypeRectangle;

typedef struct _GeometryTypeCircle {
	Vec2F off;
	float r;
} GeometryTypeCircle;

typedef union _GeometryUnion {
	GeometryTypeRectangle rect;
	GeometryTypeCircle circ;
} GeometryUnion;

typedef struct _Geometry {
	GeometryType type;
	GeometryUnion opt;
	int layer; // The layer index of the object geometry
	struct _Geometry *next;
} Geometry;

#endif
