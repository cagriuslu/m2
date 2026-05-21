#pragma once
#include "../SplitCharacter.h"
#include <m2g_VariableType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using VariableTypes = std::array<m2g::pb::VariableType, N>;

	/// A VariableValue container that can hold a set of reserved VariableTypes and a subset of VariableTypes
	template <VariableTypes reservedVariableTypes, VariableTypes subsetVariableTypes = VariableTypes<0>{}, std::size_t maxSubsetSize = 0>
	class StrictVariableContainer {
		static_assert(AreArrayElementsUnique(reservedVariableTypes), "Reserved VariableTypes are not unique");
		static_assert(AreArrayElementsUnique(subsetVariableTypes), "Subset VariableTypes are not unique");
		static_assert(AreArrayElementsUnique(ConcatArray(reservedVariableTypes, subsetVariableTypes)), "Reserved and subset VariableTypes contain common elements");
		static_assert(maxSubsetSize <= subsetVariableTypes.size());

		static constexpr int ReservedVariableTypeIndex(const m2g::pb::VariableType vt) {
			for (int i = 0; i < I(reservedVariableTypes.size()); ++i) { if (reservedVariableTypes[i] == vt) { return i; } }
			return -1;
		}
		static constexpr bool SubsetVariableTypeContains(const m2g::pb::VariableType vt) {
			for (int i = 0; i < I(subsetVariableTypes.size()); ++i) { if (subsetVariableTypes[i] == vt) { return true; } }
			return false;
		}

		std::array<VariableValue, reservedVariableTypes.size()> _reservedVariables;
		std::array<std::pair<m2g::pb::VariableType, VariableValue>, maxSubsetSize> _subsetVariables;

	public:
		template <m2g::pb::VariableType variableType>
		[[nodiscard]] static constexpr bool CanHoldVariable() {
			if constexpr (constexpr auto reservedIndex = ReservedVariableTypeIndex(variableType); reservedIndex != -1) {
				return true;
			}
			return SubsetVariableTypeContains(variableType);
		}
		[[nodiscard]] static bool CanHoldVariable(const m2g::pb::VariableType vt) {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				return true;
			}
			return SubsetVariableTypeContains(vt);
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] const VariableValue& GetVariable() const {
			static_assert(CanHoldVariable<variableType>(), "Character can't hold the given VariableType");
			if constexpr (constexpr auto reservedIndex = ReservedVariableTypeIndex(variableType); reservedIndex != -1) {
				return _reservedVariables[reservedIndex];
			} else {
				for (const auto& v : _subsetVariables) { if (v.first == variableType) { return v.second; } }
				return NULL_VARIABLE_VALUE;
			}
		}
		[[nodiscard]] const VariableValue& GetVariable(const m2g::pb::VariableType vt) const {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				return _reservedVariables[reservedIndex];
			} else {
				for (const auto& v : _subsetVariables) { if (v.first == vt) { return v.second; } }
				return NULL_VARIABLE_VALUE;
			}
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] VariableValue* MutableVariable() const {
			static_assert(CanHoldVariable<variableType>(), "Character can't hold the given VariableType");
			if constexpr (constexpr auto reservedIndex = ReservedVariableTypeIndex(variableType); reservedIndex != -1) {
				return &_reservedVariables[reservedIndex];
			} else {
				for (auto& v : _subsetVariables) { if (v.first == variableType) { return &v.second; } }
				for (auto& v : _subsetVariables) { if (not v.first) { v.first = variableType; v.second = {}; return &v.second; } }
				return nullptr;
			}
		}
		[[nodiscard]] VariableValue* MutableVariable(const m2g::pb::VariableType vt) const {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				return &_reservedVariables[reservedIndex];
			} else {
				for (auto& v : _subsetVariables) { if (v.first == vt) { return &v.second; } }
				for (auto& v : _subsetVariables) { if (not v.first) { v.first = vt; v.second = {}; return &v.second; } }
				return nullptr;
			}
		}

		template <m2g::pb::VariableType variableType>
		VariableValue SetVariable(const VariableValue varVal) {
			constexpr auto reservedIndex = ReservedVariableTypeIndex(variableType);
			static_assert(reservedIndex != -1, "Character can't statically store the given VariableType");
			_reservedVariables[reservedIndex] = varVal;
			return varVal;
		}
		[[nodiscard]] bool TrySetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				_reservedVariables[reservedIndex] = varVal;
				return true;
			}
			for (auto& v : _subsetVariables) {
				if (v.first == vt) {
					v.second = varVal;
					return true;
				}
			}
			for (auto& v : _subsetVariables) {
				if (not v.first) {
					v.first = vt;
					v.second = varVal;
					return true;
				}
			}
			return false;
		}
		void UnsafeSetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				_reservedVariables[reservedIndex] = varVal;
				return;
			}
			for (auto& v : _subsetVariables) {
				if (v.first == vt) {
					v.second = varVal;
					return;
				}
			}
			for (auto& v : _subsetVariables) {
				if (not v.first) {
					v.first = vt;
					v.second = varVal;
					return;
				}
			}
			if (SubsetVariableTypeContains(vt)) {
				throw M2_ERROR("Character has no space left for VariableType");
			} else {
				throw M2_ERROR("Character can't hold the given VariableType");
			}
		}

		template <m2g::pb::VariableType variableType>
		void ClearVariable() {
			constexpr auto reservedIndex = ReservedVariableTypeIndex(variableType);
			static_assert(reservedIndex != -1, "Character can't statically store the given VariableType");
			_reservedVariables[reservedIndex] = {};
		}
		void ClearVariable(const m2g::pb::VariableType vt) {
			if (const auto reservedIndex = ReservedVariableTypeIndex(vt); reservedIndex != -1) {
				_reservedVariables[reservedIndex] = {};
				return;
			}
			for (auto& v : _subsetVariables) {
				if (v.first == vt) {
					v.second = {};
					return;
				}
			}
		}

		void ClearVariables() {
			for (auto& v : _reservedVariables) {
				v = {};
			}
			for (auto& v : _subsetVariables) {
				v = {};
			}
		}

		[[nodiscard]] int32_t HashVariables(int32_t hash) const {
			for (int i = 0; i < I(reservedVariableTypes.size()); ++i) {
				const auto variableType = reservedVariableTypes[i];
				if (_reservedVariables[i]) {
					hash = HashI(variableType, hash);
					hash = _reservedVariables[i].Hash(hash);
				}
			}
			for (const auto& v : _subsetVariables) {
				if (v.first && v.second) {
					hash = HashI(v.first, hash);
					hash = v.second.Hash(hash);
				}
			}
			return hash;
		}
		void FillVariables(pb::LockstepDebugStateReport::Character::Variables& variablesReport) const {
			for (int i = 0; i < I(reservedVariableTypes.size()); ++i) {
				const auto vt = reservedVariableTypes[i];
				if (_reservedVariables[i]) {
					variablesReport.mutable_variable()->emplace(vt, static_cast<pb::VariableValue>(_reservedVariables[i]));
				}
			}
			for (const auto& v : _subsetVariables) {
				if (v.first && v.second) {
					variablesReport.mutable_variable()->emplace(v.first, static_cast<pb::VariableValue>(v.second));
				}
			}
		}
		void StoreVariables(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
			for (int i = 0; i < I(reservedVariableTypes.size()); ++i) {
				const auto vt = reservedVariableTypes[i];
				if (_reservedVariables[i]) {
					auto* var = objDesc.add_variables();
					var->set_type(vt);
					var->mutable_var_val()->CopyFrom(static_cast<pb::VariableValue>(_reservedVariables[i]));
				}
			}
			for (const auto& v : _subsetVariables) {
				if (v.first && v.second) {
					auto* var = objDesc.add_variables();
					var->set_type(v.first);
					var->mutable_var_val()->CopyFrom(static_cast<pb::VariableValue>(v.second));
				}
			}
		}
		void LoadVariables(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
			_reservedVariables = {};
			_subsetVariables = {};
			for (const auto& v : objDesc.variables()) {
				UnsafeSetVariable(v.type(), VariableValue{v.var_val()});
			}
		}
	};
}
