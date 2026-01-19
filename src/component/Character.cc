#include <m2/component/Character.h>
#include <m2/Game.h>
#include <algorithm>
#include <m2/protobuf/Detail.h>

using namespace m2;

Character::Character(uint64_t object_id) : Component(object_id) {}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(Character& initiator, std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, &initiator, data);
	}
	return {};
}
std::unique_ptr<const Proxy::InterCharacterMessage> Character::ExecuteInteraction(std::unique_ptr<const Proxy::InterCharacterMessage>&& data) {
	if (this->onMessage) {
		return this->onMessage(*this, nullptr, data);
	}
	return {};
}

int32_t Character::AddVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> maxValue) {
	if (GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains FE");
	}
	if (maxValue) {
		SetVariable(vt, std::min(GetVariable(vt).GetIntOrZero() + value, *maxValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetIntOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::AddVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> maxValue) {
	if (GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains int32");
	}
	if (maxValue) {
		SetVariable(vt, std::min(GetVariable(vt).GetFEOrZero() + value, *maxValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetFEOrZero() + value);
	}
	return GetVariable(vt).UnsafeGetFE();
}
int32_t Character::SubtractVariable(const m2g::pb::VariableType vt, const int32_t value, const std::optional<int32_t> minValue) {
	if (GetVariable(vt).IsFE()) {
		throw M2_ERROR("Variable contains FE");
	}
	if (minValue) {
		SetVariable(vt, std::max(GetVariable(vt).GetIntOrZero() - value, *minValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetIntOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetInt();
}
FE Character::SubtractVariable(const m2g::pb::VariableType vt, const FE value, const std::optional<FE> minValue) {
	if (GetVariable(vt).IsInt()) {
		throw M2_ERROR("Variable contains int32");
	}
	if (minValue) {
		SetVariable(vt, std::max(GetVariable(vt).GetFEOrZero() - value, *minValue));
	} else {
		SetVariable(vt, GetVariable(vt).GetFEOrZero() - value);
	}
	return GetVariable(vt).UnsafeGetFE();
}
