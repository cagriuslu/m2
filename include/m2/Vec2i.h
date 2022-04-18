#ifndef M2_VEC2I_H
#define M2_VEC2I_H

#include <cstdint>
#include <cstddef>

namespace m2 {
	struct Vec2f;
	
	struct Vec2i {
		int32_t x, y;

		Vec2i();
		Vec2i(int32_t x, int32_t y);
		Vec2i(float x, float y);
		explicit Vec2i(const Vec2f& v);

		Vec2i operator+(const Vec2i& rhs) const;
		Vec2i operator-(const Vec2i& rhs) const;
		bool operator==(const Vec2i& other) const;
		explicit operator bool() const;
	};

	struct Vec2iHash {
		size_t operator()(const Vec2i& a) const;
	};
}

#endif //M2_VEC2I_H
