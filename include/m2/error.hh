#ifndef M2_ERROR_HH
#define M2_ERROR_HH

#include <m2/Def.hh>
#include <stdexcept>

namespace m2 {
    struct error : public std::runtime_error {
        M2Err code;

        explicit error(M2Err code);
    };
}

#endif //M2_ERROR_HH
