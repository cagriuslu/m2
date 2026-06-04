#include <m2g/objects/Player.h>
#include <m2/Game.h>
#include <m2g_ObjectType.pb.h>

m2g::object::Player::Player(m2::Object& object) : character::PlayerCharacterBase(object.GetId()) {}

m2g::object::Player& m2g::object::Player::Create(const bool thisInstance, std::vector<m2::ObjectId>& multiPlayerObjectIds) {
	auto objIt = m2::CreateObject(pb::PLAYER);
	auto& chr = M2_LEVEL.AddCharacterToObject<ProxyEx::PlayerCharacterStorageIndex>(*objIt, *objIt);

	auto& phy = objIt->AddPhysique();
	if (thisInstance) {
		phy.preStep = [](m2::Physique&, const m2::Stopwatch::Duration&) {};
	}

	multiPlayerObjectIds.emplace_back(objIt.GetId());
	return chr;
}
