#pragma once
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include "subjourneys/POISelectionJourney.h"
#include "Common.h"

m2::void_expected can_player_attempt_to_sell(m2::Character& player);

enum class SellJourneyStep {
	INITIAL_STEP = 0,
	EXPECT_INDUSTRY_LOCATION,
	EXPECT_MERCHANT_LOCATION,
	EXPECT_RESOURCE_SOURCE,
	EXPECT_DEVELOP_BENEFIT_INDUSTRY_TILE,
	EXPECT_CONFIRMATION,
};
class SellJourney : public m2::FsmBase<SellJourneyStep, POIOrCancelSignal> {
	Card _selected_card{};
	IndustryLocation _selected_location{};
	MerchantLocation _merchant_location{};
	std::vector<Location> _beer_sources;
	std::vector<m2::Object*> _reserved_beers;
	IndustryTile _merchant_develop_benefit_industry_tile{};

public:
	SellJourney();
	~SellJourney() override;

protected:
	std::optional<SellJourneyStep> handle_signal(const POIOrCancelSignal& s) override;
	std::optional<SellJourneyStep> handle_initial_enter_signal();
	std::optional<SellJourneyStep> handle_industry_location_enter_signal();
	std::optional<SellJourneyStep> handle_industry_location_poi_or_cancel_signal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> handle_industry_location_exit_signal();
	std::optional<SellJourneyStep> handle_merchant_location_enter_signal();
	std::optional<SellJourneyStep> handle_merchant_location_poi_or_cancel_signal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> handle_merchant_location_exit_signal();
	std::optional<SellJourneyStep> handle_resource_enter_signal();
	std::optional<SellJourneyStep> handle_resource_poi_or_cancel_signal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> handle_resource_exit_signal();
	std::optional<SellJourneyStep> handle_develop_benefit_industry_tile_enter_signal();
	std::optional<SellJourneyStep> handle_confirmation_enter_signal();

private:
	[[nodiscard]] Industry selected_industry() const;
};

m2::void_expected can_player_sell(m2::Character& player, const m2g::pb::ClientCommand_SellAction& sell_action);

Card execute_sell_action(m2::Character& player, const m2g::pb::ClientCommand_SellAction& sell_action);
