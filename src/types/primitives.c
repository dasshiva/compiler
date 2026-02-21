#include "primitives.h"
#include <stddef.h>

enum BUILTIN_TAGS {
	TAG_I8, TAG_I16, TAG_I32, TAG_I64,
	TAG_U8, TAG_U16, TAG_U32, TAG_U64,
	TAG_MAX
};

const int BUILTIN_TAGS_MAX = TAG_MAX;

// BTD.flags:
// Bit 0-1 - 00 if signed
// 			 01 if unsigned
// 			 11 if not applicable
//
// Bit 2 - 1 if all unary ops are supported
// 		   0 if not
//
// Bit 3 - 1 if all binary ops are supported
// 		   0 if not

typedef struct BuiltinTypeData {
	int  kind;   				// kind of builtin type
	int  flags;					// flags associated with the built-in type
	int  len_operators;			// length of operators
	int* operators;				// array of OperatorCode-s supported
	int  len_compat_type_tags;  // length of compat_type_tags
	int* compat_type_tags;		// tags of types that we can convert to
} BTD;

enum BTDKind {
	BTD_KIND_INTEGRAL = 0
};

#define TYPE_SIGNED (0)
#define TYPE_UNSIGNED (1 << 0)
#define TYPE_SIGN_NA (2) 

#define TYPE_ALL_UNOPS_SUPPORTED (1 << 2)
#define TYPE_ALL_BINOPS_SUPPORTED (1 << 3)

// Do not include i8 here - a type is obviously compatible with itself
static int i8_compat_type_tags[] = { TAG_I16, TAG_I32, TAG_I64 };
static BTD BTD_I8 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_SIGNED | TYPE_ALL_UNOPS_SUPPORTED | 
		TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 0,
	.operators = NULL,
	.len_compat_type_tags = 3,
	.compat_type_tags = i8_compat_type_tags
};

static Type TYPE_I8 = { 
	.name = "i8", 
	.size = 1, 
	.align = 1, 
	.tag = TAG_I8,
	.data = &BTD_I8
};

static int i16_compat_type_tags[] = { TAG_I32, TAG_I64 };
static BTD BTD_I16 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_SIGNED | TYPE_ALL_UNOPS_SUPPORTED | 
		TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 0,
	.operators = NULL,
	.len_compat_type_tags = 2,
	.compat_type_tags = i16_compat_type_tags
};

static Type TYPE_I16 = { 
	.name = "i16", 
	.size = 2, 
	.align = 2, 
	.tag = TAG_I16,
	.data = &BTD_I16
};

static int i32_compat_type_tags[] = { TAG_I64 };
static BTD BTD_I32 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_SIGNED | TYPE_ALL_UNOPS_SUPPORTED | 
		TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 0,
	.operators = NULL,
	.len_compat_type_tags = 1,
	.compat_type_tags = i32_compat_type_tags
};

static Type TYPE_I32 = { 
	.name = "i32", 
	.size = 4, 
	.align = 4, 
	.tag = TAG_I32,
	.data = &BTD_I32
};

static BTD BTD_I64 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_SIGNED | TYPE_ALL_UNOPS_SUPPORTED | 
		TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 0,
	.operators = NULL,
	.len_compat_type_tags = 0,
	.compat_type_tags = NULL
};

static Type TYPE_I64 = { 
	.name = "i64", 
	.size = 8, 
	.align = 8, 
	.tag = TAG_I64,
	.data = &BTD_I64
};

static int utype_unary_ops[] = { OP_UNARY_ADD };
static int u8_compat_type_tags[] = { TAG_U16, TAG_U32, TAG_U64 };
static BTD BTD_U8 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_UNSIGNED | TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 1,
	.operators = utype_unary_ops,
	.len_compat_type_tags = 3,
	.compat_type_tags = u8_compat_type_tags
};

static Type TYPE_U8 = { 
	.name = "u8", 
	.size = 1, 
	.align = 1, 
	.tag = TAG_U8,
	.data = &BTD_U8
};

static int u16_compat_type_tags[] = { TAG_U32, TAG_U64 };
static BTD BTD_U16 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_UNSIGNED | TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 1,
	.operators = utype_unary_ops,
	.len_compat_type_tags = 2,
	.compat_type_tags = u16_compat_type_tags
};

static Type TYPE_U16 = { 
	.name = "u16", 
	.size = 2, 
	.align = 2, 
	.tag = TAG_U16,
	.data = &BTD_U16
};

static int u32_compat_type_tags[] = { TAG_U64 };
static BTD BTD_U32 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_UNSIGNED | TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 1,
	.operators = utype_unary_ops,
	.len_compat_type_tags = 1,
	.compat_type_tags = u32_compat_type_tags
};

static Type TYPE_U32 = { 
	.name = "u32", 
	.size = 4, 
	.align = 4, 
	.tag = TAG_U32,
	.data = &BTD_U32
};

static BTD BTD_U64 = {
	.kind = BTD_KIND_INTEGRAL,
	.flags = TYPE_UNSIGNED | TYPE_ALL_BINOPS_SUPPORTED,
	.len_operators = 1,
	.operators = utype_unary_ops,
	.len_compat_type_tags = 0,
	.compat_type_tags = NULL
};

static Type TYPE_U64 = { 
	.name = "u64", 
	.size = 8, 
	.align = 8, 
	.tag = TAG_U64,
	.data = &BTD_U64
};

Type* I8() { return &TYPE_I8; }
Type* I16() { return &TYPE_I16; }
Type* I32() { return &TYPE_I32; }
Type* I64() { return &TYPE_I64; }

Type* U8() { return &TYPE_U8; }
Type* U16() { return &TYPE_U16; }
Type* U32() { return &TYPE_U32; }
Type* U64() { return &TYPE_U64; }

Type* BUILTIN_TYPES[] = {
	&TYPE_I8, &TYPE_I16, &TYPE_I32, &TYPE_I64,
	&TYPE_U8, &TYPE_U16, &TYPE_U32, &TYPE_U64
};

const int len_builtins = 8;

int PrimitiveSupportsOp(Type* type, OperatorCode code, int arity) {
	BTD* btd = type->data;
	if (arity == 1) {
		if (btd->flags & TYPE_ALL_UNOPS_SUPPORTED)
			return 1;
		for (int i = 0; i < btd->len_operators; i++) {
			if (btd->operators[i] == code)
				return 1;
		}

		return 0;
	}

	if (btd->flags & TYPE_ALL_BINOPS_SUPPORTED)
		return 1;

	for (int i = 0; i < btd->len_operators; i++) {
		if (btd->operators[i] == code)
			return 1;
	}

	return 0;
}
