#pragma once
#include <stdexcept>
#include <string>

#define M2_ERROR(msg) (::m2::Error(__FILE__, __LINE__, (msg)))

namespace m2 {
    struct Error : public std::runtime_error {
        Error(const char* file, int line, const std::string& msg);
        Error(const char* file, int line, const char* msg);

        static void SetLogger(void (*logger)(const char* file, int line, const std::string& msg));

    private:
        static void (*_logger)(const char* file, int line, const std::string& msg);
    };
}
