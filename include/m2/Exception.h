#ifndef M2_EXCEPTION_H
#define M2_EXCEPTION_H

#include <stdexcept>
#include <string>

#define M2FATAL(msg) (::m2::Fatal(__FILE__, __LINE__, (msg)))
#define M2ERROR(msg) (::m2::Error(__FILE__, __LINE__, (msg)))

namespace m2 {
    struct Fatal : public std::runtime_error {
		Fatal(const char* file, int line, const std::string& msg);
        Fatal(const char* file, int line, const char* msg);
    };

    struct Error : public std::runtime_error {
        Error(const char* file, int line, const std::string& msg);
        Error(const char* file, int line, const char* msg);
    };
}

#endif //M2_EXCEPTION_H
