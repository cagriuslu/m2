#pragma once
#include <cuzn/Detail.h>
#include <m2/Fsm.h>
#include "Common.h"

enum class LiquidationJourneyStep {
	EXPECT_LOCATION = 0
};

class LiquidationJourney : public m2::FsmBase<LiquidationJourneyStep, PositionOrCancelSignal> {
	int _money_to_be_paid;
	std::vector<IndustryLocation> _selected_locations;

public:
	explicit LiquidationJourney(int money_to_be_paid);

protected:
	std::optional<LiquidationJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<LiquidationJourneyStep> handle_location_enter_signal();
	std::optional<LiquidationJourneyStep> handle_location_mouse_click_signal(const m2::VecF&);
	std::optional<LiquidationJourneyStep> handle_location_exit_signal();
};