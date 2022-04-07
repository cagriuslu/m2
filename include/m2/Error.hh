#ifndef M2_ERROR_HH
#define M2_ERROR_HH

#include <m2/Def.h>
#include <stdexcept>

#define M2FATAL(...) (::m2::Fatal(__FILE__, __LINE__, __VA_ARGS__))
#define M2ERROR(...) (::m2::Error(__FILE__, __LINE__, __VA_ARGS__))

namespace m2 {
    struct Fatal : public std::runtime_error {
        Fatal(const char* file, int line, M2Err code);
    };

    struct Error : public std::runtime_error {
        Error(const char* file, int line, M2Err code);
    };
}

#endif //M2_ERROR_HH
