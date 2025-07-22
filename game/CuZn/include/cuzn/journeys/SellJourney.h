#pragma once
#include <cuzn/Detail.h>
#include <m2/game/Fsm.h>
#include "subjourneys/POISelectionJourney.h"
#include "Common.h"
#include <Network.pb.h>

m2::void_expected CanPlayerAttemptToSell(m2::Character& player);

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

	std::optional<POISelectionJourney> sub_journey;

protected:
	std::optional<SellJourneyStep> HandleSignal(const POIOrCancelSignal& s) override;
	std::optional<SellJourneyStep> HandleInitialEnterSignal();
	std::optional<SellJourneyStep> HandleIndustryLocationEnterSignal();
	std::optional<SellJourneyStep> HandleIndustryLocationPoiOrCancelSignal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> HandleIndustryLocationExitSignal();
	std::optional<SellJourneyStep> HandleMerchantLocationEnterSignal();
	std::optional<SellJourneyStep> HandleMerchantLocationPoiOrCancelSignal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> HandleMerchantLocationExitSignal();
	std::optional<SellJourneyStep> HandleResourceEnterSignal();
	std::optional<SellJourneyStep> HandleResourcePoiOrCancelSignal(const POIOrCancelSignal&);
	std::optional<SellJourneyStep> HandleResourceExitSignal();
	std::optional<SellJourneyStep> HandleDevelopBenefitIndustryTileEnterSignal();
	std::optional<SellJourneyStep> HandleConfirmationEnterSignal();

private:
	[[nodiscard]] Industry selected_industry() const;
};

m2::void_expected CanPlayerSell(m2::Character& player, const m2g::pb::TurnBasedClientCommand_SellAction& sell_action);

Card ExecuteSellAction(m2::Character& player, const m2g::pb::TurnBasedClientCommand_SellAction& sell_action);
