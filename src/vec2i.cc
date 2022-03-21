#include <m2/vec2i.hh>
#include <m2/Vec2F.h>
#include <unordered_map>

m2::vec2i::vec2i() : x(0), y(0) {

}
m2::vec2i::vec2i(int32_t x, int32_t y) : x(x), y(y) {

}
m2::vec2i::vec2i(float x, float y) : x(static_cast<int32_t>(x)), y(static_cast<int32_t>(y)) {

}
m2::vec2i::vec2i(const m2::vec2f& v) : vec2i(v.x, v.y) {
	
}

m2::vec2i m2::vec2i::operator+(const vec2i& rhs) const {
	return {x + rhs.x, y + rhs.y};
}
m2::vec2i m2::vec2i::operator-(const vec2i& rhs) const {
	return {x - rhs.x, y - rhs.y};
}
bool m2::vec2i::operator==(const vec2i& other) const {
	return x == other.x && y == other.y;
}
m2::vec2i::operator bool() const {
	return (x || y);
}

size_t m2::vec2i_hash::operator()(const vec2i& a) const {
	uint64_t packed = static_cast<uint64_t>(a.x) | (static_cast<uint64_t>(a.y) << 32);
	return std::hash<uint64_t>{}(packed);
}
