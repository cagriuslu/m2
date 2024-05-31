#pragma once
#include "Common.h"
#include <cuzn/Detail.h>
#include <m2/Fsm.h>
#include <m2/VecF.h>
#include <m2/Object.h>

enum class BuildJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};

class BuildJourney : public m2::FsmBase<BuildJourneyStep, PositionOrCancelSignal> {
	Card _selected_card{};
	IndustryLocation _selected_location{};
	Industry _selected_industry{};
	IndustryTile _industry_tile{};
	std::vector<std::pair<m2g::pb::ResourceType, Location>> _resource_sources;
	std::vector<std::pair<m2::Object*, m2g::pb::ResourceType>> _reserved_resources;

public:
	BuildJourney();

protected:
	std::optional<BuildJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<BuildJourneyStep> handle_initial_enter_signal();
	std::optional<BuildJourneyStep> handle_location_enter_signal();
	std::optional<BuildJourneyStep> handle_location_mouse_click_signal(const m2::VecF&);
	std::optional<BuildJourneyStep> handle_location_cancel_signal();
	std::optional<BuildJourneyStep> handle_location_exit_signal();
	std::optional<BuildJourneyStep> handle_resource_enter_signal();
	std::optional<BuildJourneyStep> handle_resource_mouse_click_signal(const m2::VecF&);
	std::optional<BuildJourneyStep> handle_resource_cancel_signal();
	std::optional<BuildJourneyStep> handle_resource_exit_signal();
	std::optional<BuildJourneyStep> handle_confirmation_enter_signal();

	decltype(_resource_sources)::iterator get_next_unspecified_resource();
};
