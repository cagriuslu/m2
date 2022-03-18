#ifndef M2_VEC2F_HH
#define M2_VEC2F_HH

namespace m2 {
	struct vec2i;

	struct vec2f {
		float x, y;

		vec2f();
		vec2f(float x, float y);
		vec2f(int x, int y);
		explicit vec2f(const vec2i& v);

		vec2f operator+(const vec2f& rhs) const;
		vec2f operator-(const vec2f& rhs) const;
		vec2f operator*(const float& rhs) const;
		vec2f operator/(const float& rhs) const;
		bool operator==(const vec2f& other) const;
		explicit operator bool() const;

		[[nodiscard]] bool is_nan() const;
		[[nodiscard]] float length() const;
		[[nodiscard]] float length_sq() const;
		[[nodiscard]] float distance(const vec2f& other) const;
		[[nodiscard]] float distance_sq(const vec2f& other) const;
		[[nodiscard]] float angle_rads() const;

		[[nodiscard]] vec2f normalize() const;
		[[nodiscard]] vec2f floor_length(float len) const;
		[[nodiscard]] vec2f with_length(float len) const;
		[[nodiscard]] vec2f ceil_length(float len) const;
		[[nodiscard]] vec2f lerp(const vec2f& to, float ratio) const;

		static vec2f nan();
		static vec2f from_angle(float rads);
	};
}

#endif //M2_VEC2F_HH
