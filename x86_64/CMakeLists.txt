file(GLOB X86_64_SOURCES "src/*.s")
add_library(x86_64 include/x86_64.h ${X86_64_SOURCES})
target_include_directories(x86_64 PUBLIC include)