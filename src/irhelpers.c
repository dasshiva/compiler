#include "irgenhelpers.h"
#include "counter.h"

#include <stdlib.h>
static Counter counter;

static IRInst* MakeIRInst(enum IRInstruction inst) {
	IRInst* ret = malloc(sizeof(IRInst));
	if (!ret)
		return NULL;

	ret->code = inst;
	return ret;
}

uint32_t IRConst(Vector* IR, IRType* type, int64_t n) {
	IRInst* ret = MakeIRInst(IR_CONST);
	IRConstant* ct = malloc(sizeof(IRConstant));
	ct->type = type;
	ct->target = n;
	ct->ID = GetCounter(&counter);

	ret->operands = ct;
	Append(IR, ret);

	return ct->ID;
}

static uint32_t IRInitBinaryOp(Vector* IR, enum IRInstruction inst, 
		uint32_t left, uint32_t right, IRType* type) {

	IRInst* ret = MakeIRInst(inst);
	IRBinaryOp* op = malloc(sizeof(IRBinaryOp));
	op->type = type;
	op->left = left;
	op->right = right;
	op->ID = GetCounter(&counter);

	ret->operands = op;
	Append(IR, ret);
	return op->ID;
}

uint32_t IRAdd(Vector* IR, uint32_t left, uint32_t right, IRType* type) {
	return IRInitBinaryOp(IR, IR_ADD, left, right, type);
}


uint32_t IRSub(Vector* IR, uint32_t left, uint32_t right, IRType* type) {
	return IRInitBinaryOp(IR, IR_SUB, left, right, type);
}

uint32_t IRMul(Vector* IR, uint32_t left, uint32_t right, IRType* type) {
	return IRInitBinaryOp(IR, IR_MUL, left, right, type);
}

uint32_t IRDiv(Vector* IR, uint32_t left, uint32_t right, IRType* type) {
	return IRInitBinaryOp(IR, IR_DIV, left, right, type);
}

uint32_t IRMod(Vector* IR, uint32_t left, uint32_t right, IRType* type) {
	return IRInitBinaryOp(IR, IR_MODULUS, left, right, type);
}

uint32_t IRNeg(Vector* IR, uint32_t operand, IRType* type) {
	IRInst* inst = MakeIRInst(IR_NEG);
	IRNegate* op = malloc(sizeof(IRNegate));
	op->ID = GetCounter(&counter);
	op->type = type;
	op->target = operand;

	inst->operands = op;
	Append(IR, inst);
	return op->ID;
}

uint32_t NewCounter() {
	return GetCounter(&counter);
}
