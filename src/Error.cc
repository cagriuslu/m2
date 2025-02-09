#include <m2/Error.h>
#include <m2/Log.h>

m2::Error::Error(const char* file, const int line, const std::string& msg) : std::runtime_error(msg) {
	detail::Log(pb::LogLevel::ERR, file, line, msg.c_str());
}
m2::Error::Error(const char* file, const int line, const char* msg) : Error(file, line, std::string{msg}) {}
