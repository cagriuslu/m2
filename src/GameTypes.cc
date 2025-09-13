#include <m2/GameTypes.h>

m2::IFE::IFE(const pb::IFE& ife) {
	if (ife.has_i()) {
		_value = ife.i();
	} else if (ife.has_fe()) {
		_value = FE::FromProtobufRepresentation(ife.fe().value_e6());
	} else {
		_value = 0;
	}
}
