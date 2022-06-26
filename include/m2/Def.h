#ifndef DEFS_H
#define DEFS_H

#include <m2/Vec2f.h>
#include <variant>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <stddef.h>

////////////////////////////////////////////////////////////////////////
////////////////////////////// Definitions /////////////////////////////
////////////////////////////////////////////////////////////////////////

#define M2_2PI  6.283185307179586f // 2pi
#define M2_PI   3.141592653589793f // pi
#define M2_PI2  1.570796326794897f // pi/2
#define M2_PI4  0.785398163397448f // pi/4
#define M2_3PI4 2.356194490192345f // 3pi/4

namespace m2 {
	using ID = uint64_t;
	using SpriteIndex = uint32_t;
	// TODO create custom classes so that we can provide to_string methods
}

////////////////////////////////////////////////////////////////////////
///////////////////////// Platform Abstraction /////////////////////////
////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define STRDUP _strdup
#else
#define STRDUP strdup
#endif

////////////////////////////////////////////////////////////////////////
////////////////////////// Convenience Macros //////////////////////////
////////////////////////////////////////////////////////////////////////

#define DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	size_t SiblingListLength_##typeName(const typeName *ptr)
#define DEFINE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) \
	DECLARE_SIBLING_LIST_LENGTH_CALCULATOR(typeName) { size_t len; for (len = 0; ptr; len++, ptr = ptr->next); return len; }

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif
