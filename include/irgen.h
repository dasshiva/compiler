#ifndef __IRGEN_H__
#define __IRGEN_H__

#include "sema.h"

enum IRInstruction {
	IR_NEW, IR_STORE, IR_LOAD, IR_ADD, IR_SUB,
	IR_MUL, IR_DIV, IR_MODULUS, IR_MAX
};

enum OperandType {
	OP_IMM, OP_RESOURCE
};

typedef struct Operand {
	enum OperandType type;
	union {
		int imm;
		int resource;
	};
} Operand;

typedef struct IRInst {
	enum IRInstruction inst;
	uint32_t  flags;
	int  id;
	Operand operands[3];
} IRInst;

Vector* GenIR(Vector* stats, Vector* symtab);
void PrintIR(Vector* IR);

#endif
