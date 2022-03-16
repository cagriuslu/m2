#ifndef STRING_H
#define STRING_H

#include <m2/Def.h>
#include <m2/Array.h>

M2Err String_Split(const char* string, char delimiter, Array* out);

#endif
