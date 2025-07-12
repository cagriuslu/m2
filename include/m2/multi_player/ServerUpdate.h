#pragma once
#include <Network.pb.h>

namespace m2 {
	/// nextSequenceNo is incremented while generating the ServerUpdate
	m2::pb::NetworkMessage GenerateServerUpdate(int32_t& nextSequenceNo, int turnHolderIndex, bool shutdown);
}
