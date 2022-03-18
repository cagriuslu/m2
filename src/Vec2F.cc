#include "m2/Vec2F.hh"
#include <math.h>

static float _Vec2F_Length(Vec2F in) {
	return sqrtf(in.x * in.x + in.y * in.y);
}

static float _Vec2F_LengthSquared(Vec2F in) {
	return in.x * in.x + in.y * in.y;
}

Vec2F Vec2F_Add(Vec2F lhs, Vec2F rhs) {
	return Vec2F{lhs.x + rhs.x, lhs.y + rhs.y};
}

Vec2F Vec2F_Sub(Vec2F lhs, Vec2F rhs) {
	return Vec2F{lhs.x - rhs.x, lhs.y - rhs.y};
}

Vec2F Vec2F_Mul(Vec2F lhs, float rhs) {
	return Vec2F{lhs.x * rhs, lhs.y * rhs};
}

Vec2F Vec2F_Div(Vec2F lhs, float rhs) {
	return Vec2F{ lhs.x / rhs, lhs.y / rhs };
}

Vec2F Vec2F_Normalize(Vec2F in) {
	if (in.x == 0.0f && in.y == 0.0f) {
		return VEC2F_ZERO;
	}
	float len = _Vec2F_Length(in);
	if (len == 0.0f) {
		return VEC2F_ZERO;
	}
	return Vec2F{ in.x / len, in.y / len };
}

Vec2F Vec2F_SetLength(Vec2F in, float len) {
	Vec2F normalized = Vec2F_Normalize(in);
	return Vec2F_Mul(normalized, len);
}

Vec2F Vec2F_FloorLength(Vec2F in, float len) {
	float inLen = _Vec2F_Length(in);
	return Vec2F_SetLength(in, inLen < len ? len : inLen);
}

Vec2F Vec2F_CeilLength(Vec2F in, float len) {
	float inLen = _Vec2F_Length(in);
	return Vec2F_SetLength(in, len < inLen ? len : inLen);
}

float Vec2F_Distance(Vec2F lhs, Vec2F rhs) {
	return _Vec2F_Length(Vec2F_Sub(lhs, rhs));
}

float Vec2F_DistanceSquared(Vec2F lhs, Vec2F rhs) {
	return _Vec2F_LengthSquared(Vec2F_Sub(lhs, rhs));
}

Vec2F Vec2F_Lerp(Vec2F from, Vec2F to, float ratio) {
	Vec2F diffVec = Vec2F_Sub(to, from);
	return Vec2F_Add(from, Vec2F_Mul(diffVec, ratio));
}

float Vec2F_AngleRads(Vec2F vector) {
	return atan2f(vector.y, vector.x);
}

Vec2F Vec2F_FromAngle(float rads) {
	return Vec2F{cosf(rads), sinf(rads)};
}

bool Vec2F_Equals(Vec2F lhs, Vec2F rhs) {
	return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

bool Vec2F_IsNan(Vec2F in) {
	return isnan(in.x) || isnan(in.y);
}

Vec2F Vec2F_FromVec2I(Vec2I v) {
	return Vec2F{ (float)v.x, (float)v.y };
}
