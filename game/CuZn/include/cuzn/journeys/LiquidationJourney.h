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
	std::optional<LiquidationJourneyStep> handle_signal(const POIOrCancelSignal& s) override;
	std::optional<LiquidationJourneyStep> handle_location_enter_signal();
	std::optional<LiquidationJourneyStep> handle_location_mouse_click_signal(const POIOrCancelSignal&);
	std::optional<LiquidationJourneyStep> handle_location_exit_signal();
};
