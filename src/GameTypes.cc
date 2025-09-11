#include <m2/GameTypes.h>

m2::IFF::IFF(const pb::IFF& iff) {
	if (iff.has_i()) {
		_value = iff.i();
	} else if (iff.has_ff()) {
		_value = FF::FromProtobufRepresentation(iff.ff().value_e6());
	} else {
		_value = 0;
	}
}
