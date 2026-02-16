#if 0
#include "types.h"

enum IntrinsicFlags {
	COMPILE_TIME_CONST_PARAMS_ONLY = (1 << 0),
	INTRINSIC_IMPLEMENTED = (1 << 1),
};

typedef int (*IntrinsicImpl)(void*);

typedef struct ParamDescriptor {
	int param; 		// Index of the parameter
	int len;        // length of the types array
	Type** types;   // vector of all allowed types
};

typedef struct Intrinsic {
	const char* name;    		// name of the intrinsic
	Type*  ret; 	     		// the return type of the intrinsic
	ParamDescriptor** params;   // parameters of the intrinsic
	int    lparams;      		// length of Intrinsic->params
	IntrinsicImpl impl;  		// function pointer to the intrinsic if implemented
	int flags;           		// flags describing the intrinsic
} Intrinsic;

ParamDescriptor i8Params = {
	.param = 0,
	.len = 4,
	.types = { &TYPE_I8, &TYPE_I16, &TYPE_I32, &TYPE_I64 }
};

Intrinsic i8 = {
	.name = "i8",
	.ret = &TYPE_I8,
	.lparams = 1,
	.params = &i8Params,
	.impl = NULL,
	.flags = 0
};

#endif
