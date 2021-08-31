#ifndef DEFS_H
#define DEFS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))

#ifdef _WIN32
#define STRDUP _strdup
#else
#define STRDUP strdup
#endif

#endif
