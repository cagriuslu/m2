#include <m2/error.hh>

m2::error::error(M2Err code) : code(code), std::runtime_error(M2Err_ToString(code)) {

}
