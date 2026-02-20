#include "types.h"

enum BUILTIN_TAGS {
	TAG_I8, TAG_I16, TAG_I32, TAG_I64,
	TAG_U8, TAG_U16, TAG_U32, TAG_U64,
	TAG_MAX
};

const int BUILTIN_TAGS_MAX = TAG_MAX;

enum IntTypeFlags {
	INT_SIGNED, INT_UNSIGNED
};

static Type TYPE_I8 = { 
	.name = "i8", 
	.size = 1, 
	.align = 1, 
	.tag = TAG_I8,
	.flags = INT_SIGNED
};

static Type TYPE_I16 = { 
	.name = "i16", 
	.size = 2, 
	.align = 2, 
	.tag = TAG_I16,
	.flags = INT_SIGNED
};

static Type TYPE_I32 = { 
	.name = "i32", 
	.size = 4, 
	.align = 4, 
	.tag = TAG_I32,
	.flags = INT_SIGNED
};

static Type TYPE_I64 = { 
	.name = "i64", 
	.size = 8, 
	.align = 8, 
	.tag = TAG_I64,
	.flags = INT_SIGNED	
};

static Type TYPE_U8 = { 
	.name = "u8", 
	.size = 1, 
	.align = 1, 
	.tag = TAG_U8,
	.flags = INT_UNSIGNED
};

static Type TYPE_U16 = { 
	.name = "u16", 
	.size = 2, 
	.align = 2, 
	.tag = TAG_U16,
	.flags = INT_UNSIGNED
};

static Type TYPE_U32 = { 
	.name = "u32", 
	.size = 4, 
	.align = 4, 
	.tag = TAG_U32,
	.flags = INT_UNSIGNED
};

static Type TYPE_U64 = { 
	.name = "u64", 
	.size = 8, 
	.align = 8, 
	.tag = TAG_U64,
	.flags = INT_UNSIGNED
};

Type* I8() { return &TYPE_I8; }
Type* I16() { return &TYPE_I16; }
Type* I32() { return &TYPE_I32; }
Type* I64() { return &TYPE_I64; }

Type* U8() { return &TYPE_U8; }
Type* U16() { return &TYPE_U16; }
Type* U32() { return &TYPE_U32; }
Type* U64() { return &TYPE_U64; }

int TypeSupportsOp(Type* type, OperatorCode code, Type* stype) {
	int arity = OperatorArity(code);
	
	// All currently defined types support all unary operators
	if (arity == 1)
		return 1;

	// If both types are equal, no need to check thoroughly
	if (type == stype)
		return 1;

	return type->flags == stype->flags;
}

Type* BUILTIN_TYPES[] = {
	&TYPE_I8, &TYPE_I16, &TYPE_I32, &TYPE_I64,
	&TYPE_U8, &TYPE_U16, &TYPE_U32, &TYPE_U64
};

const int len_builtins = 8;
