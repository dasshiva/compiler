#ifndef __IRGENHELPERS_H__
#define __IRGENHELPERS_H__

#include "irgen.h"

uint32_t IRConst(Vector* IR, IRType* type, int64_t n); 
uint32_t IRAdd(Vector* IR, uint32_t left, uint32_t right, IRType* type);
uint32_t IRSub(Vector* IR, uint32_t left, uint32_t right, IRType* type);
uint32_t IRMul(Vector* IR, uint32_t left, uint32_t right, IRType* type);
uint32_t IRDiv(Vector* IR, uint32_t left, uint32_t right, IRType* type);
uint32_t IRMod(Vector* IR, uint32_t left, uint32_t right, IRType* type); 

uint32_t NewCounter();
#endif
