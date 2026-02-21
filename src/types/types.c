#include "types.h"
#include "primitives.h"

int TypeSupportsOp(Type* type, OperatorCode code) {
	if (type->tag >= BUILTIN_TAGS_MAX)
		return -1;

	int arity = OperatorArity(code);
	if (!PrimitiveSupportsOp(type, code, arity))
		return 0;

	return 1;
}


