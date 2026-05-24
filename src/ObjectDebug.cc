#include <m2/ObjectDebug.h>
#include <m2/Game.h>
#include <m2/Object.h>
#include <format>

using namespace m2;

void ObjectDebugOptions::ForEachMonitorValue(const CharacterStorage& chrStorage, const Object& obj, const std::function<void(const std::string&)>& op) const {
	if (const auto chrId = obj.GetCharacterId()) {
		for (const auto& cardTypeCountMonitor : characterMonitor.cardTypeCount) {
			const auto count = chrStorage.CountCards(chrId, cardTypeCountMonitor);
			op(std::format("{} count: {}", pb::enum_name(cardTypeCountMonitor), count ? *count : 0));
		}
		for (const auto& cardCategoryCountMonitor : characterMonitor.cardCategoryCount) {
			const auto count = chrStorage.CountCards(chrId, cardCategoryCountMonitor);
			op(std::format("{} count: {}", pb::enum_name(cardCategoryCountMonitor), count ? *count : 0));
		}
		for (const auto& firstCardTypeMonitor : characterMonitor.firstCardType) {
			const auto card = chrStorage.GetFirstCardType(chrId, firstCardTypeMonitor);
			op(std::format("First {}: {}", pb::enum_name(firstCardTypeMonitor), card ? pb::enum_name(*card) : "<NONE>"));
		}
		for (const auto& variableValueMonitor : characterMonitor.variableValue) {
			const auto& value = chrStorage.GetVariable(chrId, variableValueMonitor);
			op(std::format("{} value: {}", pb::enum_name(variableValueMonitor), value ? ToString(*value) : "<NONE>"));
		}
		for (const auto& customStateMonitor : characterMonitor.customStateMonitor) {
			if (customStateMonitor.second) {
				const auto value = customStateMonitor.second();
				const auto str = std::format("{}: {}", customStateMonitor.first, ToString(value));
				op(str);
			}
		}
	}
}
