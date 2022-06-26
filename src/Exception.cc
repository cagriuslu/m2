#include <m2/Exception.h>
#include <m2/Log.h>

m2::Fatal::Fatal(const char* file, int line, const char* msg) : std::runtime_error(msg) {
	internal::log(LogLevel::Fatal, file, line, msg);
}

m2::Error::Error(const char* file, int line, const char* msg) : std::runtime_error(msg) {
	internal::log(LogLevel::Error, file, line, msg);
}
