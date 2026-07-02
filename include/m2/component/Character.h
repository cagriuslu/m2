#pragma once
#include "../Component.h"
#include "../Card.h"
#include <m2/math/VariableValue.h>
#include <m2/Proxy.h>
#include <m2/reflect/Composite.h>
#include <m2/reflect/Primitive.h>
#include <Lockstep.pb.h>
#include <algorithm>
#include <functional>
#include <optional>

namespace m2 {
	namespace detail {
		enum class SomeScopedEnum { Default };
		using SomeReflectiveVariant = reflect::Variant<reflect::Field<SomeScopedEnum::Default, reflect::Empty>>;
	}

	using Interaction = std::unique_ptr<Proxy::InterCharacterMessage>;

	/// Even though each object has the option to store their data in impl, there are some benefits to storing the data
	/// inside the character: (1) In turn-based games, the character states are shared automatically. (2) In lockstep
	/// games, the synchronization of the clients are compared automatically. (3) Enables automatic saving and loading
	/// of levels.

	/// Define the requirements from a character implementation from the game engine perspective
	template <typename T>
	concept CharacterImpl = requires(T t) {
		requires ComponentImpl<T>;
		{ t.OnUpdate(std::declval<Stopwatch::Duration>()) } -> std::same_as<void>;
		{ t.OnMessage(std::declval<Interaction>()) } -> std::same_as<void>;
		{ t.OnMessage(detail::SomeReflectiveVariant{}) } -> std::same_as<void>;

		{ std::as_const(t).Hash(std::declval<int32_t>()) } -> std::same_as<int32_t>;
		{ std::as_const(t).Fill(std::declval<pb::LockstepDebugStateReport::Character&>()) } -> std::same_as<void>;
		{ std::as_const(t).Store(std::declval<pb::TurnBasedServerUpdate::ObjectDescriptor&>()) } -> std::same_as<void>;
		{ t.Load(std::declval<const pb::TurnBasedServerUpdate::ObjectDescriptor&>()) } -> std::same_as<void>;

		{ std::as_const(t).CountCards(std::declval<m2g::pb::CardType>()) } -> std::same_as<int>;
		{ std::as_const(t).CountCards(std::declval<m2g::pb::CardCategory>()) } -> std::same_as<int>;
		{ std::as_const(t).GetFirstCardType(std::declval<m2g::pb::CardCategory>()) } -> std::same_as<m2g::pb::CardType>;
		{ std::as_const(t).GetVariable(std::declval<m2g::pb::VariableType>()) } -> std::same_as<VariableValue>;
	};

	// Utilities

	template <CharacterImpl C>
	int32_t UnsafeAddVariable(C& c, const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> maxValue = {}) {
		if (c.GetVariable(vt).IsNonNull() && not c.GetVariable(vt).IsInt()) {
			throw M2_ERROR("Variable contains a different type of value");
		}
		if (maxValue) {
			c.UnsafeSetVariable(vt, std::min(c.GetVariable(vt).GetIntOrZero() + value, *maxValue));
		} else {
			c.UnsafeSetVariable(vt, c.GetVariable(vt).GetIntOrZero() + value);
		}
		return c.GetVariable(vt).UnsafeGetInt();
	}
	template <CharacterImpl C>
	FE UnsafeAddVariable(C& c, const m2g::pb::VariableType vt, const FE value, const std::optional<FE> maxValue = {}) {
		if (c.GetVariable(vt).IsNonNull() && not c.GetVariable(vt).IsFE()) {
			throw M2_ERROR("Variable contains a different type of value");
		}
		if (maxValue) {
			c.UnsafeSetVariable(vt, std::min(c.GetVariable(vt).GetFEOrZero() + value, *maxValue));
		} else {
			c.UnsafeSetVariable(vt, c.GetVariable(vt).GetFEOrZero() + value);
		}
		return c.GetVariable(vt).UnsafeGetFE();
	}
	template <CharacterImpl C, bool Enable = not GAME_IS_DETERMINISTIC>
	constexpr float UnsafeAddVariable(C& c, const m2g::pb::VariableType vt, const float value, const std::optional<float> maxValue = {}) requires (Enable) {
		return UnsafeAddVariable(c, vt, FE{value}, maxValue ? std::optional{FE{*maxValue}} : std::optional<FE>{}).ToFloat();
	}

	template <CharacterImpl C>
	int32_t UnsafeSubtractVariable(C& c, const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> minValue = {}) {
		if (c.GetVariable(vt).IsNonNull() && not c.GetVariable(vt).IsInt()) {
			throw M2_ERROR("Variable contains a different type of value");
		}
		if (minValue) {
			c.UnsafeSetVariable(vt, std::max(c.GetVariable(vt).GetIntOrZero() - value, *minValue));
		} else {
			c.UnsafeSetVariable(vt, c.GetVariable(vt).GetIntOrZero() - value);
		}
		return c.GetVariable(vt).UnsafeGetInt();
	}
	template <CharacterImpl C>
	FE UnsafeSubtractVariable(C& c, const m2g::pb::VariableType vt, const FE value, const std::optional<FE> minValue = {}) {
		if (c.GetVariable(vt).IsNonNull() && not c.GetVariable(vt).IsFE()) {
			throw M2_ERROR("Variable contains a different type of value");
		}
		if (minValue) {
			c.UnsafeSetVariable(vt, std::max(c.GetVariable(vt).GetFEOrZero() - value, *minValue));
		} else {
			c.UnsafeSetVariable(vt, c.GetVariable(vt).GetFEOrZero() - value);
		}
		return c.GetVariable(vt).UnsafeGetFE();
	}
	template <CharacterImpl C, bool Enable = not GAME_IS_DETERMINISTIC>
	constexpr float UnsafeSubtractVariable(C& c, const m2g::pb::VariableType vt, const float value, const std::optional<float> minValue = {}) requires (Enable) {
		return UnsafeSubtractVariable(c, vt, FE{value}, minValue ? std::optional{FE{*minValue}} : std::optional<FE>{}).ToFloat();
	}

	// Helpers

	[[nodiscard]] bool IsDescendantOf(ObjectId objId, ObjectId parentId);
	m2g::pb::CardCategory ToCategoryOfCard(m2g::pb::CardType);
}
