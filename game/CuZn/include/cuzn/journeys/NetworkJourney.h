#pragma once
#include "Common.h"
#include <m2g_ItemType.pb.h>
#include <m2g_ResourceType.pb.h>
#include <m2/game/Fsm.h>
#include <m2/math/VecF.h>
#include <m2/component/Character.h>
#include <cuzn/Detail.h>
#include <m2/Object.h>
#include <m2/ui/UiPanel.h>
#include <Network.pb.h>
#include <list>
#include "subjourneys/POISelectionJourney.h"

m2::void_expected CanPlayerAttemptToNetwork(m2::Character& player);

enum class NetworkJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_RESOURCE_SOURCE, // Only while building railroads, resource is required
	EXPECT_CONFIRMATION,
};

class NetworkJourney : public m2::FsmBase<NetworkJourneyStep, POIOrCancelSignal> {
	struct ResourceSource {
		Connection connection{};
		m2g::pb::ResourceType resource_type{};
		Location source{};
		m2::Object* reserved_object{};
	};

	std::optional<std::list<m2::UiPanel>::iterator> _cancel_button_panel;

	bool _build_double_railroads{};
	m2g::pb::ItemType _selected_card{};
	m2g::pb::SpriteType _selected_connection_1{}, _selected_connection_2{};
	std::vector<ResourceSource> _resource_sources;
	m2::ObjectId _decoy_road_1{}, _decoy_road_2{};

public:
	NetworkJourney();
	~NetworkJourney() override;

	std::optional<POISelectionJourney> sub_journey{};

protected:
	std::optional<NetworkJourneyStep> HandleSignal(const POIOrCancelSignal& s) override;
	std::optional<NetworkJourneyStep> HandleInitialEnterSignal();
	std::optional<NetworkJourneyStep> HandleLocationEnterSignal();
	std::optional<NetworkJourneyStep> HandleLocationMouseClickSignal(const POIOrCancelSignal&);
	std::optional<NetworkJourneyStep> HandleLocationExitSignal();
	std::optional<NetworkJourneyStep> HandleResourceEnterSignal();
	std::optional<NetworkJourneyStep> HandleResourceMouseClickSignal(const POIOrCancelSignal&);
	std::optional<NetworkJourneyStep> HandleResourceExitSignal();
	std::optional<NetworkJourneyStep> HandleConfirmationEnterSignal();

	std::vector<ResourceSource> RequiredResourcesForNetwork();
	decltype(_resource_sources)::iterator GetNextUnspecifiedResource();
};

// For the server
m2::void_expected CanPlayerNetwork(m2::Character& player, const m2g::pb::TurnBasedClientCommand_NetworkAction& network_action);

// For the server
std::pair<Card,int> ExecuteNetworkAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_NetworkAction& network_action);
