#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include <unordered_map>
#include <sstream>

m2::Vec2i::Vec2i(const m2::Vec2f& v) : Vec2i(v.x, v.y) {}

std::string m2::to_string(const m2::Vec2i& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << "}";
	return ss.str();
}
