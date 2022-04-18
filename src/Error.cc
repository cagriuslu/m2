#include <m2/Error.h>

m2::Fatal::Fatal(const char* file, int line, M2Err code) : std::runtime_error(M2Err_ToString(code)) {
    Log_M2(LogLevelFatal, file, line, code);
}

m2::Error::Error(const char* file, int line, M2Err code) : std::runtime_error(M2Err_ToString(code)) {
    Log_M2(LogLevelError, file, line, code);
}
