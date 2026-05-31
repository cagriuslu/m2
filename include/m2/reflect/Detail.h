#pragma once
#include <type_traits>

namespace m2::reflect {
	template <typename T>
	constexpr bool IsValuePackUnique(T) {
		return true; // A single element is always unique
	}
	template <typename T, typename... Args>
	constexpr bool IsValuePackUnique(T first, Args... next) {
		const bool firstIsUnique = ((first != next) && ...);
		const bool restIsUnique = IsValuePackUnique(next...);
		return firstIsUnique && restIsUnique;
	}

	using Path = std::vector<int>;
	enum class ContainerType {
		Sequence
	};
	enum class CompositeType {
		Struct,
		Variant
	};

	template <typename T>
	concept Accessor = requires(T t, const Path& path) {
		{ t(path) };
		{ t(path, true) };
		{ t(path, static_cast<uint8_t>(0)) };
		{ t(path, static_cast<int8_t>(0)) };
		{ t(path, static_cast<int16_t>(0)) };
		{ t(path, static_cast<int32_t>(0)) };
		{ t(path, static_cast<int64_t>(0)) };
		{ t(path, 0.0f) };
		{ t(path, 0.0) };
		{ t(path, ContainerType{}) };
		{ t(path, CompositeType{}) };
	};
	struct AnyAccessor {
		void operator()(const Path&) {}
		void operator()(const Path&, bool) {}
		void operator()(const Path&, uint8_t) {}
		void operator()(const Path&, int8_t) {}
		void operator()(const Path&, int16_t) {}
		void operator()(const Path&, int32_t) {}
		void operator()(const Path&, int64_t) {}
		void operator()(const Path&, float) {}
		void operator()(const Path&, double) {}
		void operator()(const Path&, ContainerType) {}
		void operator()(const Path&, CompositeType) {}
	};
	static_assert(Accessor<AnyAccessor>);

	template <typename T>
	concept IsPrimitiveReflective = requires(T t, AnyAccessor& accessor, const Path& path) {
		{ std::as_const(t).ReflectPrimitive(accessor, path) };
	};
	template <typename T>
	concept IsContainerReflective = requires(T t, AnyAccessor& accessor, Path& path) {
		{ std::as_const(t).ReflectContainer(accessor, path) };
	};
	template <typename T>
	concept IsCompositeReflective = requires(T t, AnyAccessor& accessor, Path& path) {
		{ std::as_const(t).ReflectComposite(accessor, path) };
	};
	template <typename T>
	concept IsReflective = IsPrimitiveReflective<T> || IsContainerReflective<T> || IsCompositeReflective<T>;

	/// Requires T to be enum class
	template <typename T>
	concept IsScopedEnum = std::is_enum_v<T> && !std::is_convertible_v<T, std::underlying_type_t<T>>;

	template <IsScopedEnum auto Id_, IsReflective T>
	struct Field {
		static constexpr auto Id = Id_;
		using Type = T;
	};

	template <typename T>
	class IsDerivedFromField {
		template <auto a, typename U>
		static constexpr std::true_type Test(const Field<a,U>*) { return {}; } // Accepts pointer to Field
		static constexpr std::false_type Test(...) { return {}; } // Fallback for anything else
	public:
		static constexpr bool Value = decltype(Test(std::declval<T*>()))::value; // See which overload matches
	};
}
