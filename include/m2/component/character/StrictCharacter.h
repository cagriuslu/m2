#pragma once
#include <m2/GameTypes.h>
#include <m2/Meta.h>
#include <m2g_ItemType.pb.h>
#include <m2g_VariableType.pb.h>
#include <array>

namespace m2 {
	template <std::size_t N>
	using PossibleItemTypes = std::array<m2g::pb::ItemType, N>;

	template <std::size_t N>
	using PossibleVariableTypes = std::array<m2g::pb::VariableType, N>;

	template <PossibleItemTypes possibleItemTypes, PossibleVariableTypes possibleVariableTypes>
	class StrictCharacter {
		// Verify that item types and variable types are unique
		static_assert(AreArrayElementsUnique(possibleItemTypes), "StrictCharacter supports only unique ItemTypes");
		static_assert(AreArrayElementsUnique(possibleVariableTypes), "StrictCharacter supports only unique VariableTypes");

		static consteval int ItemTypeIndex(m2g::pb::ItemType it) {
			for (int i = 0; i < possibleItemTypes.size(); ++i) {
				if (possibleItemTypes[i] == it) { return i; }
			}
			return -1;
		}
		static consteval int VariableTypeIndex(m2g::pb::VariableType vt) {
			for (int i = 0; i < possibleVariableTypes.size(); ++i) {
				if (possibleVariableTypes[i] == vt) { return i; }
			}
			return -1;
		}

		std::array<bool, possibleItemTypes.size()> _items;
		std::array<IFE, possibleVariableTypes.size()> _variables;

	public:

		template <m2g::pb::ItemType itemType>
		[[nodiscard]] bool HasItem() const {
			static_assert(DoesArrayContainElement(possibleItemTypes, itemType), "This StrictCharacter specialization can't hold the given ItemType");
			return _items[ItemTypeIndex(itemType)];
		}
		template <m2g::pb::ItemType itemType>
		void AddItemIfNotPresent() {
			static_assert(DoesArrayContainElement(possibleItemTypes, itemType), "This StrictCharacter specialization can't hold the given ItemType");
			_items[ItemTypeIndex(itemType)] = true;
		}
		template <m2g::pb::ItemType itemType>
		void RemoveItem() {
			static_assert(DoesArrayContainElement(possibleItemTypes, itemType), "This StrictCharacter specialization can't hold the given ItemType");
			_items[ItemTypeIndex(itemType)] = false;
		}

		template <m2g::pb::VariableType variableType>
		[[nodiscard]] IFE GetVariable() const {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			return _variables[VariableTypeIndex(variableType)];
		}
		template <m2g::pb::VariableType variableType>
		IFE SetVariable(const IFE ife) {
			static_assert(DoesArrayContainElement(possibleVariableTypes, variableType), "This StrictCharacter specialization can't hold the given VariableType");
			_variables[VariableTypeIndex(variableType)] = ife; return ife;
		}
	};
}
