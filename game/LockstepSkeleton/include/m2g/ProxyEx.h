#pragma once
#include <m2g/objects/Player.h>
#include <m2/ProxyEx.h>

namespace m2g {
	class ProxyEx final : public m2::ProxyEx {
	public:
		using CharacterVariants = std::tuple<object::Player>;
		static constexpr auto PlayerCharacterStorageIndex = m2::GetIndexInTuple<object::Player, CharacterVariants>::value;
	};
}
