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

int TypesCompatible(Type* t1, Type* t2) {
	if (t1->tag >= BUILTIN_TAGS_MAX || t2->tag >= BUILTIN_TAGS_MAX)
		return -1;

	if (!PrimitiveTypesCompatible(t1, t2))
		return 0;

	return 1;
}
