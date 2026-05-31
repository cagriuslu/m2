#pragma once
#include "Detail.h"
#include <cstdint>

namespace m2::reflect {
	class Bool {
		bool _value{};

	public:
		Bool() = default;
		explicit Bool(const bool v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Bool>);

	class Byte {
		uint8_t _value{};

	public:
		Byte() = default;
		explicit Byte(const uint8_t v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Byte>);

	class Int8 {
		int8_t _value{};

	public:
		Int8() = default;
		explicit Int8(const int8_t v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Int8>);

	class Int16 {
		int16_t _value{};

	public:
		Int16() = default;
		explicit Int16(const int16_t v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Int16>);

	class Int32 {
		int32_t _value{};

	public:
		Int32() = default;
		explicit Int32(const int32_t v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Int32>);

	class Int64 {
		int64_t _value{};

	public:
		Int64() = default;
		explicit Int64(const int64_t v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Int64>);

	class Float {
		float _value{};

	public:
		Float() = default;
		explicit Float(const float v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Float>);

	class Double {
		double _value{};

	public:
		Double() = default;
		explicit Double(const double v) : _value(v) {}
		auto Get() const { return _value; }
		auto& Mutate() { return _value; }

		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path, _value); }
	};
	static_assert(IsPrimitiveReflective<Double>);

	class Empty {
	public:
		template <typename Accessor>
		void ReflectPrimitive(Accessor& accessor, const Path& path) const { accessor(path); }
	};
	static_assert(IsPrimitiveReflective<Empty>);
}
