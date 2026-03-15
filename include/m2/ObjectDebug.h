#pragma once
#include <m2g/ProxyEx.h>
#include <m2/Meta.h>
#include <functional>

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
			std::vector<std::pair<std::string, std::function<VariableValue()>>> customStateMonitor; // Name and value
		} characterMonitor;

		/// Monitors are draw above the object. This offset determines how high the monitor text should start.
		float monitorOffsetM{1.0f};

		[[nodiscard]] int GetMonitorCount() const { return
			I(characterMonitor.cardTypeCount.size()
			+ characterMonitor.cardCategoryCount.size()
			+ characterMonitor.firstCardType.size()
			+ characterMonitor.variableValue.size()
			+ characterMonitor.customStateMonitor.size()); }

		void ForEachMonitorValue(const Object&, const std::function<void(const std::string&)>&) const;
	};
}
