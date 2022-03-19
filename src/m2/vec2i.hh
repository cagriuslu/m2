#ifndef M2_VEC2I_HH
#define M2_VEC2I_HH

#include <cstdint>
#include <cstddef>

namespace m2 {
	struct vec2f;
	
	struct vec2i {
		int32_t x, y;

		vec2i();
		vec2i(int32_t x, int32_t y);
		vec2i(float x, float y);
		explicit vec2i(const vec2f& v);

		vec2i operator+(const vec2i& rhs) const;
		vec2i operator-(const vec2i& rhs) const;
		bool operator==(const vec2i& other) const;
		explicit operator bool() const;
	};

	struct vec2i_hash {
		size_t operator()(const vec2i& a) const;
	};
}

#endif //M2_VEC2I_HH
