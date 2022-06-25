#include <m2/Vec2i.h>
#include <m2/Vec2f.h>
#include <unordered_map>
#include <sstream>

m2::Vec2i::Vec2i() : x(0), y(0) {

}
m2::Vec2i::Vec2i(int32_t x, int32_t y) : x(x), y(y) {

}
m2::Vec2i::Vec2i(float x, float y) : x(static_cast<int32_t>(x)), y(static_cast<int32_t>(y)) {

}
m2::Vec2i::Vec2i(const m2::Vec2f& v) : Vec2i(v.x, v.y) {
	
}

m2::Vec2i m2::Vec2i::operator+(const Vec2i& rhs) const {
	return {x + rhs.x, y + rhs.y};
}
m2::Vec2i m2::Vec2i::operator-(const Vec2i& rhs) const {
	return {x - rhs.x, y - rhs.y};
}
bool m2::Vec2i::operator==(const Vec2i& other) const {
	return x == other.x && y == other.y;
}
m2::Vec2i::operator bool() const {
	return (x || y);
}

size_t m2::Vec2iHash::operator()(const Vec2i& a) const {
	uint64_t packed = static_cast<uint64_t>(a.x) | (static_cast<uint64_t>(a.y) << 32);
	return std::hash<uint64_t>{}(packed);
}

std::string m2::to_string(const m2::Vec2i& v) {
	std::stringstream ss;
	ss << "{ x: " << v.x << ", y: " << v.y << " }";
	return ss.str();
}
