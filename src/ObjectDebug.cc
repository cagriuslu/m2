#include <m2/ObjectDebug.h>
#include <m2/Object.h>
#include <format>

using namespace m2;

void ObjectDebugOptions::ForEachMonitorValue(const Object& obj, const std::function<void(const std::string&)>& op) const {
	if (const auto* chr = obj.TryGetCharacter()) {
		for (const auto& cardTypeCountMonitor : characterMonitor.cardTypeCount) {
			const auto count = chr->CountCards(cardTypeCountMonitor);
			const auto str = std::format("{} count: {}", pb::enum_name(cardTypeCountMonitor), count);
			op(str);
		}
		for (const auto& cardCategoryCountMonitor : characterMonitor.cardCategoryCount) {
			const auto count = chr->CountCards(cardCategoryCountMonitor);
			const auto str = std::format("{} count: {}", pb::enum_name(cardCategoryCountMonitor), count);
			op(str);
		}
		for (const auto& firstCardTypeMonitor : characterMonitor.firstCardType) {
			const auto card = chr->GetFirstCardType(firstCardTypeMonitor);
			const auto str = std::format("First {}: {}", pb::enum_name(firstCardTypeMonitor), card ? pb::enum_name(*card) : "<NONE>");
			op(str);
		}
		for (const auto& variableValueMonitor : characterMonitor.variableValue) {
			const auto& value = chr->GetVariable(variableValueMonitor);
			const auto str = std::format("{} value: {}", pb::enum_name(variableValueMonitor), ToString(value));
			op(str);
		}
	}
}
