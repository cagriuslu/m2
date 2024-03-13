#pragma once
#include <m2/Fsm.h>
#include <m2/VecF.h>
#include <m2/Object.h>

namespace cuzn {
	enum class BuildJourneyStep {
		INITIAL_STEP = 0,
		EXPECT_INDUSTRY_LOCATION,
		EXPECT_RESOURCE_SOURCE,
		EXPECT_CONFIRMATION,
	};

	class BuildJourneySignal : public m2::FsmSignalBase {
		std::optional<m2::VecF> _world_position;
		bool _cancel{};

	public:
		using m2::FsmSignalBase::FsmSignalBase;
		static BuildJourneySignal create_mouse_click_signal(m2::VecF world_position);
		static BuildJourneySignal create_cancel_signal(bool cancelled = true);

		// Accessors
		[[nodiscard]] const m2::VecF* world_position() const { return _world_position ? &*_world_position : nullptr; }
		[[nodiscard]] bool cancel() const { return _cancel; }
	};

	class BuildJourney : public m2::FsmBase<BuildJourneyStep, BuildJourneySignal> {
		m2g::pb::ItemType _selected_card{};
		m2g::pb::SpriteType _selected_location{};
		m2g::pb::ItemType _selected_industry{};
		std::vector<std::pair<m2g::pb::ResourceType, m2g::pb::SpriteType>> _resource_sources;

	public:
		BuildJourney();

	protected:
		std::optional<BuildJourneyStep> handle_signal(const BuildJourneySignal& s) override;
		std::optional<BuildJourneyStep> handle_initial_enter_signal();
		std::optional<BuildJourneyStep> handle_industry_location_enter_signal();
		std::optional<BuildJourneyStep> handle_industry_location_mouse_click_signal(const m2::VecF&);
		std::optional<BuildJourneyStep> handle_industry_location_cancel_signal();
		std::optional<BuildJourneyStep> handle_industry_location_exit_signal();
		std::optional<BuildJourneyStep> handle_confirmation_enter_signal();
		std::optional<BuildJourneyStep> handle_confirmation_result(bool cancelled);
	};
}
