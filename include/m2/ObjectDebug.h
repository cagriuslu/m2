#pragma once
#include <m2/Meta.h>

namespace m2 {
	struct Object;

	struct ObjectDebugOptions {
		bool logging{};

		struct PhysiqueMonitor {} physiqueMonitor{};

		struct GraphicMonitor {} graphicMonitor{};

		struct CharacterMonitor {
			std::vector<m2g::pb::CardType> cardTypeCount{};
			std::vector<m2g::pb::CardCategory> cardCategoryCount{};
			std::vector<m2g::pb::CardCategory> firstCardType{};
			std::vector<m2g::pb::VariableType> variableValue{};
		} characterMonitor;

		/// Monitors are draw above the object. This offset determines how high the monitor text should start.
		float monitorOffsetM{1.0f};

		[[nodiscard]] int GetMonitorCount() const { return
			I(characterMonitor.cardTypeCount.size()
			+ characterMonitor.cardCategoryCount.size()
			+ characterMonitor.firstCardType.size()
			+ characterMonitor.variableValue.size()); }

		void ForEachMonitorValue(const Object&, const std::function<void(const std::string&)>&) const;
	};
}
