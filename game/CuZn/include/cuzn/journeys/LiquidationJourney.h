#pragma once
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include "Common.h"
#include "subjourneys/POISelectionJourney.h"

enum class LiquidationJourneyStep {
	EXPECT_LOCATION = 0
};

class LiquidationJourney : public m2::FsmBase<LiquidationJourneyStep, POIOrCancelSignal> {
	int _money_to_be_paid;
	std::vector<IndustryLocation> _selected_locations;

public:
	explicit LiquidationJourney(int money_to_be_paid);
	~LiquidationJourney() override { deinit(); }

	std::optional<POISelectionJourney> sub_journey{};

protected:
	std::optional<LiquidationJourneyStep> HandleSignal(const POIOrCancelSignal& s) override;
	std::optional<LiquidationJourneyStep> HandleLocationEnterSignal();
	std::optional<LiquidationJourneyStep> HandleLocationMouseClickSignal(const POIOrCancelSignal&);
	std::optional<LiquidationJourneyStep> HandleLocationExitSignal();
};
