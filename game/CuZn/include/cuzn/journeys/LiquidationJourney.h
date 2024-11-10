#pragma once
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include "Common.h"

enum class LiquidationJourneyStep {
	EXPECT_LOCATION = 0
};

class LiquidationJourney : public m2::FsmBase<LiquidationJourneyStep, PositionOrCancelSignal> {
	int _money_to_be_paid;
	std::vector<IndustryLocation> _selected_locations;

public:
	explicit LiquidationJourney(int money_to_be_paid);
	~LiquidationJourney() override { deinit(); }

protected:
	std::optional<LiquidationJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<LiquidationJourneyStep> handle_location_enter_signal();
	std::optional<LiquidationJourneyStep> handle_location_mouse_click_signal(const PositionOrCancelSignal&);
	std::optional<LiquidationJourneyStep> handle_location_exit_signal();
};
