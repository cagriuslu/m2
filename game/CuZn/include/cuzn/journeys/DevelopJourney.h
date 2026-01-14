#pragma once
#include "Common.h"
#include <cuzn/Detail.h>
#include "subjourneys/POISelectionJourney.h"
#include <m2/game/Fsm.h>
#include <m2/ui/UiPanel.h>
#include <Network.pb.h>
#include <list>

m2::void_expected CanPlayerAttemptToDevelop(m2::Character& player);

enum class DevelopJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};

class DevelopJourney : public m2::FsmBase<DevelopJourneyStep, POIOrCancelSignal> {
	std::optional<std::list<m2::UiPanel>::iterator> _cancel_button_panel;

	bool _develop_double_tiles{};
	m2g::pb::CardType _selected_card{};
	m2g::pb::CardType _selected_tile_1{}, _selected_tile_2{};
	m2g::pb::SpriteType _iron_source_1{}, _iron_source_2{};
	m2::Object *_reserved_source_1{}, *_reserved_source_2{};
	
public:
	DevelopJourney();
	~DevelopJourney() override;

	std::optional<POISelectionJourney> sub_journey{};

protected:
	std::optional<DevelopJourneyStep> HandleSignal(const POIOrCancelSignal& s) override;
	std::optional<DevelopJourneyStep> HandleInitialEnterSignal();
	std::optional<DevelopJourneyStep> HandleResourceEnterSignal();
	std::optional<DevelopJourneyStep> HandleResourceMouseClickSignal(const POIOrCancelSignal&);
	std::optional<DevelopJourneyStep> HandleResourceExitSignal();
	std::optional<DevelopJourneyStep> HandleConfirmationEnterSignal();
};

// For the server
m2::void_expected CanPlayerDevelop(m2::Character& player, const m2g::pb::TurnBasedClientCommand_DevelopAction& develop_action);

// For the server
std::pair<m2g::pb::CardType,int> ExecuteDevelopAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_DevelopAction& develop_action);
