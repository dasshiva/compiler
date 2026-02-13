#ifndef __IRGENHELPERS_H__
#define __IRGENHELPERS_H__

#include "irgen.h"

IRInst* IRConst(Vector* IR, IRType* type, int64_t n); 
IRInst* IRAdd(Vector* IR, IRInst* left, IRInst* right, IRType* type);
IRInst* IRSub(Vector* IR, IRInst* left, IRInst* right, IRType* type);
IRInst* IRMul(Vector* IR, IRInst* left, IRInst* right, IRType* type);
IRInst* IRDiv(Vector* IR, IRInst* left, IRInst* right, IRType* type);
IRInst* IRMod(Vector* IR, IRInst* left, IRInst* right, IRType* type); 
IRInst* IRNeg(Vector* IR, IRInst* op, IRType* type);

#endif
