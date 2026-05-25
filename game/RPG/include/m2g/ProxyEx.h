#pragma once
#include <rpg/object/Enemy.h>
#include <rpg/object/Player.h>
#include <rpg/object/Projectile.h>
#include <rpg/object/Blade.h>
#include <m2/component/character/CompactCharacter.h>
#include <m2/component/character/FastCharacter.h>
#include <m2/ProxyEx.h>
#include <m2/Meta.h>

namespace m2g {
	class ProxyEx final : public m2::ProxyEx {
	public:
		using CharacterVariants = std::tuple<rpg::EnemyCharacter, rpg::PlayerCharacter, rpg::ProjectileCharacter, rpg::BladeCharacter, m2::CompactCharacter, m2::FastCharacter>;
		static constexpr auto EnemyCharacterStorageIndex = m2::GetIndexInTuple<rpg::EnemyCharacter, CharacterVariants>::value;
		static constexpr auto PlayerCharacterStorageIndex = m2::GetIndexInTuple<rpg::PlayerCharacter, CharacterVariants>::value;
		static constexpr auto ProjectileCharacterStorageIndex = m2::GetIndexInTuple<rpg::ProjectileCharacter, CharacterVariants>::value;
		static constexpr auto BladeCharacterStorageIndex = m2::GetIndexInTuple<rpg::BladeCharacter, CharacterVariants>::value;
		static constexpr auto CompactCharacterStorageIndex = m2::GetIndexInTuple<m2::CompactCharacter, CharacterVariants>::value;
		static constexpr auto FastCharacterStorageIndex = m2::GetIndexInTuple<m2::FastCharacter, CharacterVariants>::value;
	};
}
