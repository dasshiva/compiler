#include "irgenhelpers.h"
#include <stdlib.h>

static IRInst* MakeIRInst(enum IRInstruction inst) {
	IRInst* ret = malloc(sizeof(IRInst));
	if (!ret)
		return NULL;

	ret->code = inst;
	return ret;
}

IRInst* IRConst(Vector* IR, IRType* type, int64_t n) {
	IRInst* ret = MakeIRInst(IR_CONST);
	IRConstant* ct = malloc(sizeof(IRConstant));
	ct->type = type;
	ct->target = n;

	ret->operands = ct;
	Append(IR, ret);

	return ret;
}

static IRInst* IRInitBinaryOp(Vector* IR, enum IRInstruction inst, 
		IRInst* left, IRInst* right, IRType* type) {

	IRInst* ret = MakeIRInst(inst);
	IRBinaryOp* op = malloc(sizeof(IRBinaryOp));
	op->type = type;
	op->left = left;
	op->right = right;

	ret->operands = op;
	Append(IR, ret);

	return ret;
}

IRInst* IRAdd(Vector* IR, IRInst* left, IRInst* right, IRType* type) {
	return IRInitBinaryOp(IR, IR_ADD, left, right, type);
}


IRInst* IRSub(Vector* IR, IRInst* left, IRInst* right, IRType* type) {
	return IRInitBinaryOp(IR, IR_SUB, left, right, type);
}

IRInst* IRMul(Vector* IR, IRInst* left, IRInst* right, IRType* type) {
	return IRInitBinaryOp(IR, IR_MUL, left, right, type);
}

IRInst* IRDiv(Vector* IR, IRInst* left, IRInst* right, IRType* type) {
	return IRInitBinaryOp(IR, IR_DIV, left, right, type);
}

IRInst* IRMod(Vector* IR, IRInst* left, IRInst* right, IRType* type) {
	return IRInitBinaryOp(IR, IR_MODULUS, left, right, type);
}

IRInst* IRNeg(Vector* IR, IRInst* operand, IRType* type) {
	IRInst* inst = MakeIRInst(IR_NEG);
	IRNegate* op = malloc(sizeof(IRNegate));
	op->type = type;
	op->target = operand;

	inst->operands = op;
	Append(IR, inst);
	return inst;
}

