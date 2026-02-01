#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

typedef struct Type {
	const char* name;
	const uint8_t size;
	const uint8_t align;
} Type;

extern const Type TYPE_I8;
extern const Type TYPE_I16;
extern const Type TYPE_I32;
extern const Type TYPE_I64;

extern const Type TYPE_U8;
extern const Type TYPE_U16;
extern const Type TYPE_U32;
extern const Type TYPE_U64;

// Array of all known built-in types
extern const Type* BUILTIN_TYPES[];
extern const int len_builtins;

#endif
