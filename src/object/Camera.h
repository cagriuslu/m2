#ifndef M2_CAMERA_H
#define M2_CAMERA_H

#include <m2/object/Object.hh>

namespace m2 {
	namespace object {
		struct CameraData : public ObjectData {
			vec2f offset;

			CameraData();
		};
	}
}

#endif //M2_CAMERA_H
