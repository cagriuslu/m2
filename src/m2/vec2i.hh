#ifndef M2_VEC2I_HH
#define M2_VEC2I_HH

#include <cstdint>

namespace m2 {
	struct vec2i {
		int32_t x, y;

		vec2i();
		vec2i(int32_t x, int32_t y);
		vec2i(float x, float y);

		vec2i operator+(const vec2i& rhs) const;
		vec2i operator-(const vec2i& rhs) const;
		bool operator ==(const vec2i &other) const;
		explicit operator bool() const;
	};
}

#endif //M2_VEC2I_HH
