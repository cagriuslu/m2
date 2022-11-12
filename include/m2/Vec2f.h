#ifndef M2_VEC2F_H
#define M2_VEC2F_H

#include <Dim2f.pb.h>
#include <Vec2f.pb.h>
#include <Vec2i.pb.h>
#include <box2d/b2_math.h>
#include <string>
#include <array>

namespace m2 {
	struct Vec2i;

	struct Vec2f {
		float x, y;

		Vec2f();
		Vec2f(float x, float y);
		Vec2f(int x, int y);
		Vec2f(unsigned x, unsigned y);
		explicit Vec2f(const Vec2i& v);
		explicit Vec2f(const b2Vec2& v);
		explicit Vec2f(const pb::Dim2f& v);
		explicit Vec2f(const pb::Vec2f& v);
		explicit Vec2f(const pb::Vec2i& v);

		Vec2f operator+(const Vec2f& rhs) const;
		Vec2f operator+(const float& rhs) const;
		Vec2f& operator+=(const Vec2f& rhs);
		Vec2f operator-(const Vec2f& rhs) const;
		Vec2f operator-(const float& rhs) const;
		Vec2f operator*(const float& rhs) const;
		Vec2f operator/(const float& rhs) const;
		Vec2f operator/(const int& rhs) const;
		Vec2f operator/(const unsigned& rhs) const;
		bool operator==(const Vec2f& other) const;
		explicit operator bool() const;
		explicit operator b2Vec2() const;

		[[nodiscard]] bool is_nan() const;
		[[nodiscard]] bool is_small(float tolerance) const;
		[[nodiscard]] bool is_near(const Vec2f& other, float tolerance) const;
		[[nodiscard]] float length() const;
		[[nodiscard]] float length_sq() const;
		[[nodiscard]] float distance(const Vec2f& other) const;
		[[nodiscard]] float distance_sq(const Vec2f& other) const;
		[[nodiscard]] float angle_rads() const;

		[[nodiscard]] Vec2f normalize() const;
		[[nodiscard]] Vec2f ln() const;
		[[nodiscard]] Vec2f round() const;
		[[nodiscard]] Vec2i iround() const;
		[[nodiscard]] Vec2f floor() const;
		[[nodiscard]] Vec2i ifloor() const;
		[[nodiscard]] Vec2f ceil() const;
		[[nodiscard]] Vec2i iceil() const;
		[[nodiscard]] Vec2f floor_length(float len) const;
		[[nodiscard]] Vec2f with_length(float len) const;
		[[nodiscard]] Vec2f ceil_length(float len) const;
		[[nodiscard]] Vec2f lerp(const Vec2f& to, float ratio) const;

		/// Order of corners: Bottom-right, Bottom-left, Top-left, Top-right
		[[nodiscard]] std::array<Vec2f, 4> aabb_corners(float aabb_radius) const;

		static Vec2f nan();
		static Vec2f from_angle(float rads);
	};

	std::string to_string(const m2::Vec2f&);
}

#endif //M2_VEC2F_H
