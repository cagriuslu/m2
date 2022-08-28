#include <m2/Exception.h>
#include <m2/Log.h>

m2::Fatal::Fatal(const char* file, int line, const std::string& msg) : std::runtime_error(msg) {
	internal::log(LogLevel::Fatal, file, line, msg.c_str());
}
m2::Fatal::Fatal(const char* file, int line, const char* msg) : Fatal(file, line, std::string{msg}) {}

m2::Error::Error(const char* file, int line, const std::string& msg) : std::runtime_error(msg) {
	internal::log(LogLevel::Error, file, line, msg.c_str());
}
m2::Error::Error(const char* file, int line, const char* msg) : Error(file, line, std::string{msg}) {}
