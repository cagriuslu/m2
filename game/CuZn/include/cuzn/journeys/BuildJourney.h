#pragma once
#include "Common.h"
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include <m2/math/VecF.h>
#include <m2/Object.h>
#include <m2/ui/Panel.h>
#include <cuzn/journeys/subjourneys/POISelectionJourney.h>
#include <Network.pb.h>

m2::void_expected CanPlayerAttemptToBuild(m2::Character& player);

enum class BuildJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_CONFIRMATION,
};
class BuildJourney : public m2::FsmBase<BuildJourneyStep, POIOrCancelSignal> {
	std::optional<std::list<m2::ui::Panel>::iterator> _cancel_button_panel;

	Card _selected_card{};
	IndustryLocation _selected_location{};
	Industry _selected_industry{};
	IndustryTile _industry_tile{};
	std::vector<std::pair<m2g::pb::ResourceType, Location>> _resource_sources;
	std::vector<std::pair<m2::Object*, m2g::pb::ResourceType>> _reserved_resources;

public:
	BuildJourney();
	~BuildJourney() override;

	std::optional<POISelectionJourney> sub_journey;

protected:
	std::optional<BuildJourneyStep> HandleSignal(const POIOrCancelSignal& s) override;
	std::optional<BuildJourneyStep> HandleInitialEnterSignal();
	std::optional<BuildJourneyStep> HandleLocationEnterSignal();
	std::optional<BuildJourneyStep> HandleLocationMouseClickSignal(const POIOrCancelSignal& s);
	std::optional<BuildJourneyStep> HandleLocationExitSignal();
	std::optional<BuildJourneyStep> HandleResourceEnterSignal();
	std::optional<BuildJourneyStep> HandleResourceMouseClickSignal(const POIOrCancelSignal& s);
	std::optional<BuildJourneyStep> HandleResourceExitSignal();
	std::optional<BuildJourneyStep> HandleConfirmationEnterSignal();

	decltype(_resource_sources)::iterator GetNextUnspecifiedResource();
};

// For the server
bool CanPlayerBuild(m2::Character& player, const m2g::pb::ClientCommand_BuildAction& build_action);

// For the server
std::pair<Card,int> ExecuteBuildAction(m2::Character& player, const m2g::pb::ClientCommand_BuildAction& build_action);
