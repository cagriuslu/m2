#include <m2/Value.h>

m2::Failure<const char*> m2::failure(const char* e) {
	return Failure<const char*>(std::move(e));
}
