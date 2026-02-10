#pragma once

namespace m2 {
	struct ObjectDebugOptions {
		bool logging{};

		struct PhysiqueMonitor {} physiqueMonitor;

		struct GraphicMonitor {} graphicMonitor;

		struct CharacterMonitor {
			std::vector<m2g::pb::CardType> cardTypeCount;
			std::vector<m2g::pb::CardCategory> cardCategoryCount;
			std::vector<m2g::pb::CardCategory> firstCardType;
			std::vector<m2g::pb::VariableType> variableValue;
		} characterMonitor;
	};
}
