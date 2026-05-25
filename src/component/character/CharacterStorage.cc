#include <m2/component/character/CharacterStorage.h>
#include <m2/Object.h>

using namespace m2;

CharacterStorage::CharacterStorage() {
	std::apply([](auto&... args) {
		// Fold expression guarantees left-to-right evaluation. This will ensure that PoolIds will be consecutive.
		((args.emplace()), ...);
	}, _storageTuple);
}

int CharacterStorage::GetTotalCharacterCount() const {
	int count = 0;
	std::apply([&](const auto&... pool) { ((count += I(pool->Size())), ...); }, _storageTuple);
	return count;
}
std::optional<ObjectId> CharacterStorage::TryGetOwnerId(const CharacterId chrId) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return std::nullopt;
	}
	std::optional<ObjectId> retval;
	const auto getter = [&](auto* chr) -> bool {
		if (chr) { retval = chr->GetOwnerId(); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && getter(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
	return retval;
}
int32_t CharacterStorage::HashAll(int32_t hash) const {
	const auto hasher = [](const auto& pool, int32_t hash_) -> int32_t {
		for (const auto& chr : *pool) { hash_ = chr.Hash(hash_); }
		return hash_;
	};
	std::apply([&](const auto&... pool) { ((hash = hasher(pool, hash)), ...); }, _storageTuple);
	return hash;
}
void CharacterStorage::FillAll(const Pool<Object>& objects, pb::LockstepDebugStateReport& report) const {
	const auto filler = [&](const auto& pool) {
		for (const auto& chr : *pool) {
			auto* character = report.add_character();
			const auto& owner = objects[chr.GetOwnerId()];
			character->set_chr_id(owner.GetCharacterId());
			character->set_owner_id(chr.GetOwnerId());
			character->set_object_type(owner.GetType());
			character->set_parent_id(owner.GetParentId());
			chr.Fill(*character);
		}
	};
	std::apply([&](const auto&... pool) { ((filler(pool)), ...); }, _storageTuple);
}
void CharacterStorage::StoreAll(const Pool<Object>& objects, pb::TurnBasedServerUpdate& serverUpdate) const {
	const auto storer = [&](const auto& pool) {
		for (const auto& chr : *pool) {
			auto* objDesc = serverUpdate.add_objects_with_character();
			const auto& owner = objects[chr.GetOwnerId()];
			objDesc->set_object_id(owner.GetId());
			objDesc->mutable_position()->CopyFrom(static_cast<pb::VecF>(owner.InferPositionF()));
			objDesc->set_object_type(owner.GetType());
			objDesc->set_parent_id(owner.GetParentId());
			chr.Store(*objDesc);
		}
	};
	std::apply([&](const auto&... pool) { ((storer(pool)), ...); }, _storageTuple);
}
std::optional<int> CharacterStorage::TryCountCards(const CharacterId chrId, const m2g::pb::CardType ct) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return std::nullopt;
	}
	std::optional<int> retval;
	const auto getter = [&](auto* chr) -> bool {
		if (chr) { retval = chr->CountCards(ct); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && getter(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
	return retval;
}
std::optional<int> CharacterStorage::TryCountCards(const CharacterId chrId,  const m2g::pb::CardCategory cc) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return std::nullopt;
	}
	std::optional<int> retval;
	const auto getter = [&](auto* chr) -> bool {
		if (chr) { retval = chr->CountCards(cc); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && getter(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
	return retval;
}
std::optional<m2g::pb::CardType> CharacterStorage::TryGetFirstCardType(const CharacterId chrId, const m2g::pb::CardCategory cc) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return std::nullopt;
	}
	std::optional<m2g::pb::CardType> retval;
	const auto getter = [&](auto* chr) -> bool {
		if (chr) { retval = chr->GetFirstCardType(cc); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && getter(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
	return retval;
}
std::optional<VariableValue> CharacterStorage::TryGetVariable(const CharacterId chrId, const m2g::pb::VariableType vt) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return std::nullopt;
	}
	std::optional<VariableValue> retval;
	const auto getter = [&](auto* chr) -> bool {
		if (chr) { retval = chr->GetVariable(vt); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && getter(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
	return retval;
}

void CharacterStorage::UpdateAll(const Stopwatch::Duration delta) {
	const auto updater = [&](auto& pool) {
		for (auto& chr : *pool) { chr.OnUpdate(delta); }
	};
	std::apply([&](auto&... pool) { ((updater(pool)), ...); }, _storageTuple);
}
void CharacterStorage::DeliverMessage(const CharacterId chrId, Interaction interaction) {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return;
	}
	const auto deliverer = [&](auto* chr) -> bool {
		if (chr) { chr->OnMessage(std::move(interaction)); }
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && deliverer(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
}
void CharacterStorage::Load(const CharacterId chrId, const pb::TurnBasedServerUpdate::ObjectDescriptor& objDesc) {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		throw M2_ERROR("Given character ID doesn't belong to any pool in character storage");
	}
	const auto loader = [&](auto* chr) -> bool {
		if (chr) {
			chr->Load(objDesc);
			return true;
		} else {
			throw M2_ERROR("Character not found");
		}
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && loader(pool->Get(chrId))), ...);
		(void) poolIndex;
	}, _storageTuple);
}
void CharacterStorage::Free(const CharacterId chrId) {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return;
	}
	const auto remover = [](auto& pool, const CharacterId chrId_) -> bool {
		pool->Free(chrId_);
		return true;
	};
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		((poolIndex++ == tupleIndex && remover(pool, chrId)), ...);
		(void) poolIndex;
	}, _storageTuple);
}
void CharacterStorage::ClearAll() {
	std::apply([&](auto&... pool) { ((pool->Clear()), ...); }, _storageTuple);
}
