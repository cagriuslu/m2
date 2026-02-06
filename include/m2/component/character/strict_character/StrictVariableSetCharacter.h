#pragma once
#include "../StrictCharacter.h"
#include <m2g_VariableType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleVariableTypes = std::array<m2g::pb::VariableType, N>;

	template <PossibleVariableTypes possibleVariableTypes>
	class StrictVariableSetCharacter {
		// Verify that variable types are unique
		static_assert(AreArrayElementsUnique(possibleVariableTypes), "VariableTypes are not unique");

		static constexpr int VariableTypeIndex(const m2g::pb::VariableType vt) {
			for (int i = 0; i < I(possibleVariableTypes.size()); ++i) { if (possibleVariableTypes[i] == vt) { return i; } }
			return -1;
		}

		std::array<VariableValue, possibleVariableTypes.size()> _variables;

	public:
		[[nodiscard]] bool CanHoldVariable(const m2g::pb::VariableType vt) const {
			return VariableTypeIndex(vt) != -1;
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] VariableValue GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			return _variables[VariableTypeIndex(variableType)];
		}
		template <m2g::pb::VariableType variableType>
		VariableValue SetVariable(const VariableValue varVal) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			_variables[VariableTypeIndex(variableType)] = varVal; return varVal;
		}

		[[nodiscard]] VariableValue GetVariable(const m2g::pb::VariableType vt) const {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				return {};
			} else {
				return _variables[variableTypeIndex];
			}
		}
		[[nodiscard]] expected<VariableValue> TrySetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				return make_unexpected("Character cannot hold " + ToString(vt));
			} else {
				_variables[variableTypeIndex] = varVal;
				return varVal;
			}
		}
		VariableValue UnsafeSetVariable(const m2g::pb::VariableType vt, const VariableValue varVal) {
			_variables[VariableTypeIndex(vt)] = varVal;
			return varVal;
		}
		void ClearVariable(const m2g::pb::VariableType vt) {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex != -1) {
				_variables[variableTypeIndex] = {};
			}
		}

		[[nodiscard]] int32_t HashVariables(int32_t hash) const {
			for (const auto& variable : _variables) {
				if (variable) { hash = variable.Hash(hash); }
			}
			return hash;
		}
		void StoreVariables(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
			for (int i = 0; i < I(possibleVariableTypes.size()); ++i) {
				const auto possibleVariableType = possibleVariableTypes[i];
				if (_variables[i]) {
					auto* var = objDesc.add_variables();
					var->set_type(possibleVariableType);
					var->mutable_var_val()->CopyFrom(static_cast<pb::VariableValue>(_variables[i]));
				}
			}
		}
		void LoadVariables(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
			_variables = std::array<VariableValue, possibleVariableTypes.size()>{};
			for (const auto& variable : objDesc.variables()) {
				UnsafeSetVariable(variable.type(), VariableValue{variable.var_val()});
			}
		}
	};
}
