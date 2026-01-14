#include <m2/Proxy.h>
#include <m2g_CardType.pb.h>

m2::pb::Cards m2::Proxy::GenerateDefaultCards() {
	pb::Cards cardsEnvelope;
	auto* cards = cardsEnvelope.mutable_cards();
	cards->Add()->set_type(m2g::pb::NO_CARD);
	return cardsEnvelope;
}
