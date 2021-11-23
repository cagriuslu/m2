#ifndef STRING_H
#define STRING_H

#include "Array.h"
#include "Error.h"

XErr String_Split(const char* string, char delimiter, Array* out);

#endif
