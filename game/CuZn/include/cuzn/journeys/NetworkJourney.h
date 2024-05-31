#pragma once
#include "Common.h"
#include <m2g_ItemType.pb.h>
#include <m2g_ResourceType.pb.h>
#include <m2/Fsm.h>
#include <m2/VecF.h>

enum class NetworkJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};

class NetworkJourney : public m2::FsmBase<NetworkJourneyStep, PositionOrCancelSignal> {
	bool _build_double_railroads{};
	m2g::pb::ItemType _selected_card_1{}, _selected_card_2{};
	m2g::pb::SpriteType _selected_location_1{}, _selected_location_2{};
	std::vector<std::pair<m2g::pb::ResourceType, m2g::pb::SpriteType>> _resource_sources;

public:
	NetworkJourney();

protected:
	std::optional<NetworkJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<NetworkJourneyStep> handle_initial_enter_signal();
	std::optional<NetworkJourneyStep> handle_location_enter_signal();
	std::optional<NetworkJourneyStep> handle_location_mouse_click_signal(const m2::VecF&);
	std::optional<NetworkJourneyStep> handle_location_cancel_signal();
	std::optional<NetworkJourneyStep> handle_location_exit_signal();
	std::optional<NetworkJourneyStep> handle_confirmation_enter_signal();
};
