#include <m2/Animation.h>
#include <m2/protobuf/Detail.h>
#include <m2/Exception.h>
#include <utility>

using namespace m2g::pb;

m2::Animation::Animation(pb::Animation pb) : _animation(std::move(pb)) {
	_states.resize(pb::enum_value_count<AnimationStateType>());
	for (const auto& state : _animation.states()) {
		_states[pb::enum_index(state.type())] = state;
	}
}
