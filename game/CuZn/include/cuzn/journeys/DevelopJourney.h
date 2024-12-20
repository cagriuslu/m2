#pragma once
#include "Common.h"
#include <cuzn/Detail.h>
#include "subjourneys/POISelectionJourney.h"
#include <m2/game/Fsm.h>
#include <m2/ui/Panel.h>
#include <Network.pb.h>
#include <list>

m2::void_expected can_player_attempt_to_develop(m2::Character& player);

enum class DevelopJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};

class DevelopJourney : public m2::FsmBase<DevelopJourneyStep, POIOrCancelSignal> {
	std::optional<std::list<m2::ui::Panel>::iterator> _cancel_button_panel;

	bool _develop_double_tiles{};
	Card _selected_card{};
	m2g::pb::ItemType _selected_tile_1{}, _selected_tile_2{};
	m2g::pb::SpriteType _iron_source_1{}, _iron_source_2{};
	m2::Object *_reserved_source_1{}, *_reserved_source_2{};
	
public:
	DevelopJourney();
	~DevelopJourney() override;

	std::optional<POISelectionJourney> sub_journey{};

protected:
	std::optional<DevelopJourneyStep> handle_signal(const POIOrCancelSignal& s) override;
	std::optional<DevelopJourneyStep> handle_initial_enter_signal();
	std::optional<DevelopJourneyStep> handle_resource_enter_signal();
	std::optional<DevelopJourneyStep> handle_resource_mouse_click_signal(const POIOrCancelSignal&);
	std::optional<DevelopJourneyStep> handle_resource_exit_signal();
	std::optional<DevelopJourneyStep> handle_confirmation_enter_signal();
};

// For the server
bool can_player_develop(m2::Character& player, const m2g::pb::ClientCommand_DevelopAction& develop_action);

// For the server
std::pair<Card,int> execute_develop_action(m2::Character& player, const m2g::pb::ClientCommand_DevelopAction& develop_action);
