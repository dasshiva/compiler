#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "types.h"
int PrimitiveSupportsOp(Type* type, OperatorCode code, int arity);
int PrimitiveTypesCompatible(Type* t1, Type* t2);

#endif
