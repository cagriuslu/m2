#pragma once
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include "Common.h"

m2::void_expected can_player_attempt_to_sell(m2::Character& player);

enum class SellJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_MERCHANT_LOCATION,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};

class SellJourney : public m2::FsmBase<SellJourneyStep, PositionOrCancelSignal> {
	Card _selected_card{};
	IndustryLocation _selected_location{};
	std::vector<Location> _beer_sources;
	std::vector<m2::Object*> _reserved_beers;

public:
	SellJourney();

protected:
	std::optional<SellJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<SellJourneyStep> handle_initial_enter_signal();
	std::optional<SellJourneyStep> handle_location_enter_signal();
	std::optional<SellJourneyStep> handle_location_mouse_click_signal(const m2::VecF&);
	std::optional<SellJourneyStep> handle_location_cancel_signal();
	std::optional<SellJourneyStep> handle_location_exit_signal();
	std::optional<SellJourneyStep> handle_merchant_location_enter_signal();
	std::optional<SellJourneyStep> handle_merchant_location_mouse_click_signal(const m2::VecF&);
	std::optional<SellJourneyStep> handle_merchant_location_cancel_signal();
	std::optional<SellJourneyStep> handle_merchant_location_exit_signal();
	std::optional<SellJourneyStep> handle_resource_enter_signal();
	std::optional<SellJourneyStep> handle_resource_mouse_click_signal(const m2::VecF&);
	std::optional<SellJourneyStep> handle_resource_cancel_signal();
	std::optional<SellJourneyStep> handle_resource_exit_signal();
	std::optional<SellJourneyStep> handle_confirmation_enter_signal();
};
