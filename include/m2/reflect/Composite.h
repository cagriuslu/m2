#pragma once
#include "Detail.h"
#include <m2/Meta.h>
#include <variant>

namespace m2::reflect {
	template <typename Accessor, IsReflective T>
	void ReflectField(Accessor& accessor, Path& path, int fieldId, const T& value) {
		path.emplace_back(fieldId);
		if constexpr (IsPrimitiveReflective<T>) {
			value.ReflectPrimitive(accessor, path);
		} else if constexpr (IsContainerReflective<T>) {
			value.ReflectContainer(accessor, path);
		} else {
			value.ReflectComposite(accessor, path);
		}
		path.pop_back();
	}

	template <typename FirstField, typename... RestFields>
	class Struct {
		static_assert(IsDerivedFromField<FirstField>::Value && (IsDerivedFromField<RestFields>::Value && ...), "All fields must have been derived from Field");
		static_assert((std::is_same_v<decltype(FirstField::Id), decltype(RestFields::Id)> && ...), "Field IDs must have the same type");
		static_assert(IsValuePackUnique(FirstField::Id, RestFields::Id...), "Field IDs must be unique");
		std::tuple<typename FirstField::Type, typename RestFields::Type...> _storage;

		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		static constexpr std::size_t GetIndexOfId() {
			std::size_t index = 0;
			const bool firstFound = FirstField::Id == FieldId ? true : (++index, false);
			const bool found = firstFound || ((RestFields::Id == FieldId ? true : (++index, false)) || ...);
			return found ? index : SIZE_MAX;
		}
		template <std::size_t index>
		static constexpr decltype(FirstField::Id) GetIdOfIndex() {
			auto retval = static_cast<decltype(FirstField::Id)>(INT_MAX);
			const auto saver = [&](auto id) { retval = id; return true; };
			std::size_t i = 0;
			i++ == index && saver(FirstField::Id);
			((i++ == index && saver(RestFields::Id)), ...);
			(void) i;
			return retval;
		}

	public:
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto& Get() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Struct does not contain a field with the given FieldId");
			return std::get<index>(_storage).Get();
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& Mutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Struct does not contain a field with the given FieldId");
			return std::get<index>(_storage).Mutate();
		}

		template <typename Accessor>
		void ReflectComposite(Accessor& accessor, Path& path) const {
			accessor(path, CompositeType::Struct);
			std::apply([&](const auto&... field) {
				[&]<std::size_t... Is>(std::index_sequence<Is...>) {
					(ReflectField(accessor, path, static_cast<int>(GetIdOfIndex<Is>()), field), ...);
				}(std::make_index_sequence<sizeof...(field)>{});
			}, _storage);
		}
	};

	template <typename FirstField, typename... RestFields>
	class Variant {
		static_assert(IsDerivedFromField<FirstField>::Value && (IsDerivedFromField<RestFields>::Value && ...), "All fields must have been derived from Field");
		static_assert((std::is_same_v<decltype(FirstField::Id), decltype(RestFields::Id)> && ...), "Field IDs must have the same type");
		static_assert(IsValuePackUnique(FirstField::Id, RestFields::Id...), "Field IDs must be unique");
		std::variant<typename FirstField::Type, typename RestFields::Type...> _storage;

		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		static constexpr std::size_t GetIndexOfId() {
			std::size_t index = 0;
			const bool firstFound = FirstField::Id == FieldId ? true : (++index, false);
			const bool found = firstFound || ((RestFields::Id == FieldId ? true : (++index, false)) || ...);
			return found ? index : SIZE_MAX;
		}
		template <std::size_t index>
		static constexpr decltype(FirstField::Id) GetIdOfIndex() {
			auto retval = static_cast<decltype(FirstField::Id)>(INT_MAX);
			const auto saver = [&](auto id) { retval = id; return true; };
			std::size_t i = 0;
			i++ == index && saver(FirstField::Id);
			((i++ == index && saver(RestFields::Id)), ...);
			(void) i;
			return retval;
		}

	public:
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		bool HoldsAlternative() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			return std::holds_alternative<index>(_storage);
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto* TryGet() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			if (_storage.index() == index) {
				&std::get<index>(_storage).Get();
			} else {
				return decltype(&std::get<index>(_storage).Get()){nullptr};
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto& UnsafeGet() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field with the given FieldId");
			return std::get<index>(_storage).Get();
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto* TryMutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			if (_storage.index() == index) {
				&std::get<index>(_storage).Mutate();
			} else {
				return decltype(&std::get<index>(_storage).Mutate()){nullptr};
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& UnsafeMutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			return std::get<index>(_storage).Mutate();
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& Emplace() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			_storage.template emplace<index>();
			return std::get<index>(_storage).Mutate();
		}

		template <typename Accessor>
		void ReflectComposite(Accessor& accessor, Path& path) const {
			accessor(path, CompositeType::Variant);
			std::visit([&]<typename T>(const T& variant) {
				const auto activeIndex = GetIndexInVariant<T, decltype(_storage)>::value;
				ReflectField(accessor, path, static_cast<int>(GetIdOfIndex<activeIndex>()), variant);
			}, _storage);
		}
	};
}
