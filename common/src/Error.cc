#include <m2/common/Error.h>

void (*m2::Error::_logger)(const char*, int, const std::string&) = nullptr;

void m2::Error::SetLogger(void (*logger)(const char* file, int line, const std::string& msg)) {
	_logger = logger;
}

m2::Error::Error(const char* file, const int line, const std::string& msg) : std::runtime_error(msg) {
	if (_logger) { _logger(file, line, msg); }
}
m2::Error::Error(const char* file, const int line, const char* msg) : Error(file, line, std::string{msg}) {}
