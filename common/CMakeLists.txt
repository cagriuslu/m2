add_library(m2_common STATIC
    src/Error.cc
    src/math/Exact.cc
    src/math/Float.cc
    include/m2/common/BuildOptions.h
    include/m2/common/Constants.h
    include/m2/common/Error.h
    include/m2/common/Meta.h
    include/m2/common/math/Exact.h
    include/m2/common/math/Float.h
)
target_include_directories(m2_common PUBLIC include)
target_compile_features(m2_common PUBLIC cxx_std_23)
