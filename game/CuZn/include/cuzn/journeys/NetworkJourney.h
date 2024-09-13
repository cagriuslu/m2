#pragma once
#include "Common.h"
#include <m2g_ItemType.pb.h>
#include <m2g_ResourceType.pb.h>
#include <m2/game/Fsm.h>
#include <m2/VecF.h>
#include <m2/component/Character.h>
#include <cuzn/Detail.h>
#include <m2/Object.h>
#include <m2/ui/Panel.h>
#include <list>

m2::void_expected can_player_attempt_to_network(m2::Character& player);

enum class NetworkJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_LOCATION,
	EXPECT_RESOURCE_SOURCE, // Only while building railroads, resource is required
	EXPECT_CONFIRMATION,
};

class NetworkJourney : public m2::FsmBase<NetworkJourneyStep, PositionOrCancelSignal> {
	struct ResourceSource {
		Connection connection{};
		m2g::pb::ResourceType resource_type{};
		Location source{};
		m2::Object* reserved_object{};
	};

	std::list<m2::ui::Panel>::iterator _cancel_button_panel;

	bool _build_double_railroads{};
	m2g::pb::ItemType _selected_card{};
	m2g::pb::SpriteType _selected_connection_1{}, _selected_connection_2{};
	std::set<Connection> _buildable_connections; // Used as cache
	std::vector<ResourceSource> _resource_sources;
	m2::ObjectId _decoy_road_1{}, _decoy_road_2{};

public:
	NetworkJourney();
	~NetworkJourney() override;

protected:
	std::optional<NetworkJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
	std::optional<NetworkJourneyStep> handle_initial_enter_signal();
	std::optional<NetworkJourneyStep> handle_location_enter_signal();
	std::optional<NetworkJourneyStep> handle_location_mouse_click_signal(const m2::VecF&);
	std::optional<NetworkJourneyStep> handle_location_cancel_signal();
	std::optional<NetworkJourneyStep> handle_location_exit_signal();
	std::optional<NetworkJourneyStep> handle_resource_enter_signal();
	std::optional<NetworkJourneyStep> handle_resource_mouse_click_signal(const m2::VecF&);
	std::optional<NetworkJourneyStep> handle_resource_cancel_signal();
	std::optional<NetworkJourneyStep> handle_resource_exit_signal();
	std::optional<NetworkJourneyStep> handle_confirmation_enter_signal();

	std::vector<ResourceSource> required_resources_for_network();
	decltype(_resource_sources)::iterator get_next_unspecified_resource();
};

// For the server
bool can_player_network(m2::Character& player, const m2g::pb::ClientCommand_NetworkAction& network_action);

// For the server
std::pair<Card,int> execute_network_action(m2::Character& player, const m2g::pb::ClientCommand_NetworkAction& network_action);
