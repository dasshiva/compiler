#include "types.h"

Type TYPE_I8 = {
	.name = "i8",
	.size = 1,
	.align = 1,
};

Type TYPE_I16 = {
	.name = "i16",
	.size = 2,
	.align = 2,
};

Type TYPE_I32 = {
	.name = "i32",
	.size = 4,
	.align = 4,
};

Type TYPE_I64 = {
	.name = "i64",
	.size = 8,
	.align = 8
};

Type TYPE_U8 = {
	.name = "u8",
	.size = 1,
	.align = 1
};

Type TYPE_U16 = {
	.name = "u16",
	.size = 2,
	.align = 2
};

Type TYPE_U32 = {
	.name = "u32",
	.size = 4,
	.align = 4
};

Type TYPE_U64 = {
	.name = "u64",
	.size = 8,
	.align = 8
};

Type* BUILTIN_TYPES[] = {
	&TYPE_I8, &TYPE_I16, &TYPE_I32, &TYPE_I64,
	&TYPE_U8, &TYPE_U16, &TYPE_U32, &TYPE_U64
};

const int len_builtins = 8;

