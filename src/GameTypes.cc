#include <m2/GameTypes.h>
#include <m2/BuildOptions.h>

m2::IFE::IFE(const pb::IFE& ife) {
	if (ife.has_i()) {
		_value = ife.i();
	} else if (ife.has_f()) {
		if constexpr (GAME_IS_DETERMINISTIC) {
			throw M2_ERROR("Deterministic game shouldn't contain an IFE of type float");
		}
		_value = FE{ife.f()};
	} else if (ife.has_e6()) {
		if constexpr (not GAME_IS_DETERMINISTIC) {
			throw M2_ERROR("Non-deterministic game shouldn't contain an IFE of type exact");
		}
		_value = FE::FromProtobufRepresentation(ife.e6());
	} else {
		_value = 0;
	}
}
