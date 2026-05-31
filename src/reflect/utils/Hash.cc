#include <m2/reflect/utils/Hash.h>
#include <m2/math/Hash.h>

using namespace m2;
using namespace m2::reflect;
using namespace m2::reflect::util;
using namespace m2::reflect::util::detail;

void HashAccessor::operator()(const Path& path) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to empty is empty");
	}
	hash = HashI(path.back(), hash);
}
void HashAccessor::operator()(const Path& path, const bool b) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to bool is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(b, hash);
}
void HashAccessor::operator()(const Path& path, const uint8_t u) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to uint8 is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(u, hash);
}
void HashAccessor::operator()(const Path& path, const int8_t i) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to int8 is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(i, hash);
}
void HashAccessor::operator()(const Path& path, const int16_t i) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to int16 is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(i, hash);
}
void HashAccessor::operator()(const Path& path, const int32_t i) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to int32 is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(i, hash);
}
void HashAccessor::operator()(const Path& path, const int64_t i) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to int64 is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(i, hash);
}
void HashAccessor::operator()(const Path& path, const float f) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to float is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(f, hash);
}
void HashAccessor::operator()(const Path& path, const double d) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to double is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(d, hash);
}
void HashAccessor::operator()(const Path& path, const ContainerType ct) {
	if (path.empty()) {
		throw M2_ERROR("Implementation error: Path to container is empty");
	}
	hash = HashI(path.back(), hash);
	hash = HashI(I(ct), hash);
}
void HashAccessor::operator()(const Path& path, const CompositeType ct) {
	if (not path.empty()) {
		hash = HashI(path.back(), hash);
	}
	hash = HashI(I(ct), hash);
}
