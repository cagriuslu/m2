#pragma once
#include "../StrictCharacter.h"
#include <m2g_VariableType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleVariableTypes = std::array<m2g::pb::VariableType, N>;

	template <PossibleVariableTypes possibleVariableTypes, std::size_t maxVariableCount>
	class StrictVariableCountCharacter {
		// Verify that variable types are unique
		static_assert(AreArrayElementsUnique(possibleVariableTypes), "VariableTypes are not unique");

		std::array<std::pair<m2g::pb::VariableType, IVFE>, maxVariableCount> _variables;

	public:
		[[nodiscard]] bool CanHoldVariable(const m2g::pb::VariableType vt) const {
			for (int i = 0; i < I(possibleVariableTypes.size()); ++i) { if (possibleVariableTypes[i] == vt) { return true; } }
			return false;
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] IVFE GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			for (const auto& variable : _variables) { if (variable.first == variableType) { return variable.second; } }
			return {};
		}
		template <m2g::pb::VariableType variableType>
		expected<IVFE> TrySetVariable(const IVFE ivfe) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			for (auto& variable : _variables) { if (variable.first == variableType) { variable.second = ivfe; return ivfe; } }
			for (auto& variable : _variables) { if (not variable.first) { variable.first = variableType; variable.second = ivfe; return ivfe; } }
			return make_unexpected("No space left for VariableType");
		}
		template <m2g::pb::VariableType variableType>
		expected<IVFE> UnsafeSetVariable(const IVFE ivfe) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "Character can't hold the given VariableType");
			for (auto& variable : _variables) { if (variable.first == variableType) { variable.second = ivfe; return ivfe; } }
			for (auto& variable : _variables) { if (not variable.first) { variable.first = variableType; variable.second = ivfe; return ivfe; } }
			throw M2_ERROR("No space left for VariableType");
		}

		[[nodiscard]] IVFE GetVariable(const m2g::pb::VariableType vt) const {
			for (const auto& variable : _variables) { if (variable.first == vt) { return variable.second; } }
			return {};
		}
		[[nodiscard]] expected<IVFE> TrySetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) {
			for (auto& variable : _variables) { if (variable.first == vt) { variable.second = ivfe; return ivfe; } }
			for (auto& variable : _variables) { if (not variable.first) { variable.first = vt; variable.second = ivfe; return ivfe; } }
			return make_unexpected("No space left for VariableType");
		}
		IVFE UnsafeSetVariable(const m2g::pb::VariableType vt, const IVFE ivfe) {
			for (auto& variable : _variables) { if (variable.first == vt) { variable.second = ivfe; return ivfe; } }
			for (auto& variable : _variables) { if (not variable.first) { variable.first = vt; variable.second = ivfe; return ivfe; } }
			throw M2_ERROR("No space left for VariableType");
		}
		void ClearVariable(const m2g::pb::VariableType vt) {
			for (auto& variable : _variables) { if (variable.first == vt) { variable = {}; return; } }
		}

		[[nodiscard]] int32_t HashVariables(int32_t hash) const {
			for (const auto& variable : _variables) {
				if (variable.first && variable.second) { hash = variable.second.Hash(hash); }
			}
			return hash;
		}
		void StoreVariables(pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) const {
			for (const auto& variable : _variables) {
				if (variable.first && variable.second) {
					auto* var = objDesc.add_variables();
					var->set_type(variable.first);
					var->mutable_ivfe()->CopyFrom(static_cast<pb::IVFE>(variable.second));
				}
			}
		}
		void LoadVariables(const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
			_variables = std::array<std::pair<m2g::pb::VariableType, IVFE>, maxVariableCount>{};
			for (const auto& variable : objDesc.variables()) {
				UnsafeSetVariable(variable.type(), IVFE{variable.ivfe()});
			}
		}
	};
}
