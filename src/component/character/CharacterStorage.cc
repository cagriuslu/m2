#include <m2/component/character/CharacterStorage.h>
#include <m2/Object.h>

using namespace m2;

CharacterStorage::CharacterStorage() {
	std::apply([](auto&... args) {
		// Fold expression guarantees left-to-right evaluation. This will ensure that PoolIds will be consecutive.
		((args.emplace()), ...);
	}, _storageTuple);
}

const Character* CharacterStorage::GetCharacter(const CharacterId chrId) const {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return nullptr;
	}

	const Character* chr = nullptr;
	std::apply([&](const auto&... pool) {
		int poolIndex = 0;
		// Use a fold expression to look-up the Character in correct Pool
		(
			// Query only the pool whose index is the one we're looking for
			(poolIndex++ == tupleIndex && true ? (chr = pool->Get(chrId)) : nullptr), ...
		);
	}, _storageTuple);
	return chr;
}
Character* CharacterStorage::GetCharacter(const CharacterId chrId) {
	const auto baseShiftedPoolId = I(GetBasePoolId());
	const auto poolIdOfCharacter = I(ToPoolId(chrId));
	const auto tupleIndex = poolIdOfCharacter - baseShiftedPoolId;
	if (tupleIndex < 0 || I(std::tuple_size_v<StorageTuple>) <= tupleIndex) {
		return nullptr;
	}

	Character* chr = nullptr;
	std::apply([&](auto&... pool) {
		int poolIndex = 0;
		// Use a fold expression to look-up the Character in correct Pool
		(
			// Query only the pool whose index is the one we're looking for
			(poolIndex++ == tupleIndex && true ? (chr = pool->Get(chrId)) : nullptr), ...
		);
	}, _storageTuple);
	return chr;
}

int CharacterStorage::GetTotalCharacterCount() const {
	int count = 0;
	std::apply([&](const auto&... pool) {
		(
			(count += I(pool->Size())), ...
		);
	}, _storageTuple);
	return count;
}
int32_t CharacterStorage::HashCharacters(int32_t hash) const {
	const auto hasher = [](auto& pool, int32_t hash_) -> int32_t {
		for (const Character& chr : *pool) {
			hash_ = chr.Hash(hash_);
		}
		return hash_;
	};

	std::apply([&](const auto&... pool) {
		(
			(hash = hasher(pool, hash)), ...
		);
	}, _storageTuple);
	return hash;
}
void CharacterStorage::FillDebugStateReport(pb::LockstepDebugStateReport& report) const {
	const auto reporter = [&](auto& pool) {
		for (const Character& chr : *pool) {
			auto* character = report.add_character();
			character->set_chr_id(chr.GetOwner().GetCharacterId());
			character->set_owner_id(chr.GetOwnerId());
			character->set_object_type(chr.GetOwner().GetType());
			character->set_parent_id(chr.GetOwner().GetParentId());
			chr.Fill(*character);
		}
	};

	std::apply([&](const auto&... pool) {
		(
			(reporter(pool)), ...
		);
	}, _storageTuple);
}

void CharacterStorage::UpdateCharacters(const Stopwatch::Duration& delta) {
	const auto updater = [&](auto& pool) {
		for (Character& chr : *pool) {
			if (chr.update) {
				chr.update(chr, delta);
			}
		}
	};

	std::apply([&](auto&... pool) {
		(
			(updater(pool)), ...
		);
	}, _storageTuple);
}
void CharacterStorage::FreeCharacter(const CharacterId chrId) {
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
		(
			(poolIndex++ == tupleIndex && remover(pool, chrId)), ...
		);
	}, _storageTuple);
}
void CharacterStorage::ClearPools() {
	std::apply([&](auto&... pool) {
		(
			(pool->Clear()), ...
		);
	}, _storageTuple);
}
