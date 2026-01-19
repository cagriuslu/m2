#include <m2/component/character/CharacterVariant.h>

using namespace m2;

static_assert(std::forward_iterator<Pool<CharacterVariant>::Iterator>);

Character& m2::ToCharacterBase(CharacterVariant& v) {
	return std::visit([](auto& vv) -> Character& { return vv; }, v);
}
FastCharacter& m2::ToFastCharacter(CharacterVariant& v) {
	return std::visit(overloaded {
		[](FastCharacter& vv) -> FastCharacter& { return vv; },
		[](auto&) -> FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
const FastCharacter& m2::ToFastCharacter(const CharacterVariant& v) {
	return std::visit(overloaded {
		[](const FastCharacter& vv) -> const FastCharacter& { return vv; },
		[](auto&) -> const FastCharacter& { throw M2_ERROR("Unexpected character type"); }
	}, v);
}
