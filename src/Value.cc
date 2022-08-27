#include <m2/Value.h>

m2::Failure<std::string> m2::failure(const char* e) {
	return Failure<std::string>(std::string{e});
}

m2::Failure<std::string> m2::failure(const std::string& e) {
	return Failure<std::string>(e);
}
