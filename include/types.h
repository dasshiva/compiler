#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include "operators.h"

typedef struct Type {
	const char* name;
	int 		tag;
	int 		size;
	int 		align;
	int 		flags;
} Type;

extern const int BUILTIN_TAGS_MAX;

Type* I8();
Type* I16();
Type* I32();
Type* I64();

Type* U8();
Type* U16();
Type* U32();
Type* U64();

extern Type* BUILTIN_TYPES[];
extern const int len_builtins;
int TypeSupportsOp(Type* type, OperatorCode code, Type* stype);

#endif
