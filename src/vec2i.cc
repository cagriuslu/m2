#include <m2/vec2i.hh>
#include <m2/vec2f.hh>

m2::vec2i::vec2i() : x(0), y(0) {

}
m2::vec2i::vec2i(int32_t x, int32_t y) : x(x), y(y) {

}
m2::vec2i::vec2i(float x, float y) : x(static_cast<int32_t>(x)), y(static_cast<int32_t>(y)) {

}
m2::vec2i::vec2i(const m2::vec2f& v) : vec2i(v.x, v.y) {
	
}
m2::vec2i::vec2i(const Vec2F& v) : vec2i(v.x, v.y) {
	
}

m2::vec2i m2::vec2i::operator+(const vec2i& rhs) const {
	return {x + rhs.x, y + rhs.y};
}
m2::vec2i m2::vec2i::operator-(const vec2i& rhs) const {
	return {x - rhs.x, y - rhs.y};
}
bool m2::vec2i::operator==(const vec2i &other) const {
	return x == other.x && y == other.y;
}
m2::vec2i::operator bool() const {
	return (x || y);
}
