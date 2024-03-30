#pragma once
#include "Common.h"
#include <m2/Fsm.h>
#include <m2/VecF.h>
#include <m2/Object.h>

namespace cuzn {
	enum class BuildJourneyStep {
		INITIAL_STEP = 0,
		EXPECT_LOCATION,
		EXPECT_RESOURCE_SOURCE,
		EXPECT_CONFIRMATION,
	};

	class BuildJourney : public m2::FsmBase<BuildJourneyStep, PositionOrCancelSignal> {
		m2g::pb::ItemType _selected_card{};
		m2g::pb::SpriteType _selected_location{};
		m2g::pb::ItemType _selected_industry{};
		std::vector<std::pair<m2g::pb::ResourceType, m2g::pb::SpriteType>> _resource_sources;

	public:
		BuildJourney();

	protected:
		std::optional<BuildJourneyStep> handle_signal(const PositionOrCancelSignal& s) override;
		std::optional<BuildJourneyStep> handle_initial_enter_signal();
		std::optional<BuildJourneyStep> handle_industry_location_enter_signal();
		std::optional<BuildJourneyStep> handle_industry_location_mouse_click_signal(const m2::VecF&);
		std::optional<BuildJourneyStep> handle_industry_location_cancel_signal();
		std::optional<BuildJourneyStep> handle_industry_location_exit_signal();
		std::optional<BuildJourneyStep> handle_confirmation_enter_signal();
		std::optional<BuildJourneyStep> handle_confirmation_result(bool cancelled);
	};
}
