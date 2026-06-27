#pragma once
#include <m2/common/reflect/Detail.h>
#include <m2/common/Meta.h>
#include <variant>

namespace m2::reflect {
	template <typename Accessor, IsReflective T>
	void ReflectField(Accessor& accessor, Path& path, int fieldId, const T& value) {
		if constexpr (IsPrimitiveReflective<T>) {
			path.emplace_back(PrimitiveType{}, fieldId);
			value.ReflectPrimitive(accessor, path);
		} else if constexpr (IsContainerReflective<T>) {
			path.emplace_back(ContainerType::Sequence, fieldId);
			value.ReflectContainer(accessor, path);
		} else {
			path.emplace_back(T::Type, fieldId);
			value.ReflectComposite(accessor, path);
		}
		path.pop_back();
	}

	template <typename FirstField, typename... RestFields>
	class Struct {
		static_assert(IsDerivedFromField<FirstField>::Value && (IsDerivedFromField<RestFields>::Value && ...), "All fields must have been derived from Field");
		static_assert((std::is_same_v<decltype(FirstField::Id), decltype(RestFields::Id)> && ...), "Field IDs must have the same type");
		static_assert(IsValuePackUnique(FirstField::Id, RestFields::Id...), "Field IDs must be unique");
		using StorageTuple = std::tuple<typename FirstField::Type, typename RestFields::Type...>;
		StorageTuple _storage;

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
		static constexpr auto Type = CompositeType::Struct;

		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto& Get() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Struct does not contain a field with the given FieldId");
			using FieldType = std::tuple_element_t<index, StorageTuple>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				return std::get<index>(_storage).Get();
			} else {
				return std::get<index>(_storage);
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& Mutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Struct does not contain a field with the given FieldId");
			using FieldType = std::tuple_element_t<index, StorageTuple>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				return std::get<index>(_storage).Mutate();
			} else {
				return std::get<index>(_storage);
			}
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
		using StorageVariant = std::variant<typename FirstField::Type, typename RestFields::Type...>;
		StorageVariant _storage;

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
		static constexpr auto Type = CompositeType::Variant;

		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		[[nodiscard]] bool HoldsAlternative() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			return _storage.index() == index;
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto* TryGet() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			using FieldType = std::variant_alternative_t<index, StorageVariant>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				if (_storage.index() == index) { return &std::get<index>(_storage).Get(); }
				return nullptr;
			} else {
				if (_storage.index() == index) { return &std::get<index>(_storage); }
				return nullptr;
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		const auto& UnsafeGet() const {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field with the given FieldId");
			using FieldType = std::variant_alternative_t<index, StorageVariant>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				return std::get<index>(_storage).Get();
			} else {
				return std::get<index>(_storage);
			}
		}
		template <typename Visitor>
		decltype(auto) Visit(Visitor&& visitor) const { return std::visit(std::forward<Visitor>(visitor), _storage); }

		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto* TryMutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			using FieldType = std::variant_alternative_t<index, StorageVariant>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				if (_storage.index() == index) { return &std::get<index>(_storage).Mutate(); }
				return nullptr;
			} else {
				if (_storage.index() == index) { return &std::get<index>(_storage); }
				return nullptr;
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& UnsafeMutate() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			using FieldType = std::variant_alternative_t<index, StorageVariant>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				return std::get<index>(_storage).Mutate();
			} else {
				return std::get<index>(_storage);
			}
		}
		template <auto FieldId> requires IsScopedEnum<decltype(FieldId)>
		auto& Emplace() {
			constexpr auto index = GetIndexOfId<FieldId>();
			static_assert(index != SIZE_MAX, "Variant does not contain a field variant with the given FieldId");
			_storage.template emplace<index>();
			using FieldType = std::variant_alternative_t<index, StorageVariant>;
			if constexpr (UnwrapsToRawType<FieldType>) {
				return std::get<index>(_storage).Mutate();
			} else {
				return std::get<index>(_storage);
			}
		}
		template <typename Visitor>
		decltype(auto) Visit(Visitor&& visitor) { return std::visit(std::forward<Visitor>(visitor), _storage); }

		template <typename Accessor>
		void ReflectComposite(Accessor& accessor, Path& path) const {
			accessor(path, CompositeType::Variant);
			std::visit([&]<typename T>(const T& variant) {
				const auto activeIndex = GetIndexInVariant<T, StorageVariant>::value;
				ReflectField(accessor, path, static_cast<int>(GetIdOfIndex<activeIndex>()), variant);
			}, _storage);
		}
	};
}
