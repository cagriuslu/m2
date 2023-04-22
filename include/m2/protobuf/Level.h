#ifndef M2_PROTO_LEVEL_H
#define M2_PROTO_LEVEL_H

#include "../Vec2i.h"
#include <Level.pb.h>

namespace m2::protobuf::level {
	void shift_background_right(pb::Level& lb, const Vec2i& pos);
	void shift_foreground_right(pb::Level& lb, const Vec2i& pos);

	void shift_background_down(pb::Level& lb, const Vec2i& pos);
	void shift_foreground_down(pb::Level& lb, const Vec2i& pos);

	void shift_background_right_down(pb::Level& lb, const Vec2i& pos);
	void shift_foreground_right_down(pb::Level& lb, const Vec2i& pos);
}

#endif //M2_PROTO_LEVEL_H
