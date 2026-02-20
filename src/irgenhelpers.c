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
	ret->type = type;
	ct->target = n;

	ret->operands = ct;
	Append(IR, ret);

	return ret;
}

static IRInst* IRInitBinaryOp(Vector* IR, enum IRInstruction inst, 
		IRInst* left, IRInst* right, IRType* type) {

	IRInst* ret = MakeIRInst(inst);
	IRBinaryOp* op = malloc(sizeof(IRBinaryOp));
	ret->type = type;
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
	inst->type = type;
	op->target = operand;

	inst->operands = op;
	Append(IR, inst);
	return inst;
}

IRInst* IRCast(Vector* IR, IRInst* operand, IRType* type) {
	IRInst* inst = MakeIRInst(IR_CAST);
	IRCastType* op = malloc(sizeof(IRCastType));
	inst->type = type;
	op->target = operand;

	inst->operands = op;
	Append(IR, inst);
	return inst;
}

uint32_t* GetIDField(IRInst* inst) {
	switch (inst->code) {
		case IR_ADD: case IR_SUB: case IR_MUL: case IR_DIV:
		case IR_MODULUS: {
			IRBinaryOp* op = inst->operands;
			return &op->ID;
		}

		case IR_CONST: {
			IRConstant* cts = inst->operands;
			return &cts->ID;
		}

		case IR_NEG: {
			IRNegate* op = inst->operands;
			return &op->ID;
		}

		case IR_CAST: {
			IRCastType* cast = inst->operands;
			return &cast->ID;
		}
		default: return NULL;
	}
}

