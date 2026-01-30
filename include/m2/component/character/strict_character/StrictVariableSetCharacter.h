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

		std::array<IVFE, possibleVariableTypes.size()> _variables;

	public:
		[[nodiscard]] bool CanHoldVariable(const m2g::pb::VariableType vt) const {
			return VariableTypeIndex(vt) != -1;
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] IVFE GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			return _variables[VariableTypeIndex(variableType)];
		}
		template <m2g::pb::VariableType variableType>
		IVFE SetVariable(const IVFE ivfe) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			_variables[VariableTypeIndex(variableType)] = ivfe; return ivfe;
		}

		[[nodiscard]] IVFE GetVariable(const m2g::pb::VariableType vt) const {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				return {};
			} else {
				return _variables[variableTypeIndex];
			}
		}
		[[nodiscard]] expected<IVFE> TrySetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) {
			if (const auto variableTypeIndex = VariableTypeIndex(vt); variableTypeIndex == -1) {
				return make_unexpected("Character cannot hold " + ToString(vt));
			} else {
				_variables[variableTypeIndex] = ivfe;
				return ivfe;
			}
		}
		IVFE UnsafeSetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) {
			_variables[VariableTypeIndex(vt)] = ivfe;
			return ivfe;
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
					var->mutable_ivfe()->CopyFrom(static_cast<pb::IVFE>(_variables[i]));
				}
			}
		}
		void LoadVariables(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
			_variables = std::array<IVFE, possibleVariableTypes.size()>{};
			for (const auto& variable : objDesc.variables()) {
				UnsafeSetVariable(variable.type(), IVFE{variable.ivfe()});
			}
		}
	};
}
