#include <m2/Error.hh>

m2::Error::Error(M2Err code) : code(code), std::runtime_error(M2Err_ToString(code)) {}
