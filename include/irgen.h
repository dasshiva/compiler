#ifndef __IRGEN_H__
#define __IRGEN_H__

#include "sema.h"
#include <stdint.h>

enum IRInstruction {
	IR_ADD, IR_SUB, IR_MUL, IR_DIV, 
	IR_MODULUS, IR_CONST, IR_NEG, IR_CAST,
	IR_MAX
};

enum OperandType {
	OP_IMM, OP_RESOURCE
};

typedef struct Operand {
	enum OperandType type;
	union {
		int imm;
		uint32_t resource;
	};
} Operand;

typedef void* Instruction;
typedef Type IRType;

typedef struct IRInst {
	enum  IRInstruction code;
	IRType* type;
	Instruction operands;
} IRInst;

// {binary-op} [type] <left>, <right>
typedef struct IRBinaryOp {
	IRInst*  left;
	IRInst*  right;
	uint32_t ID;
} IRBinaryOp; 

// tID = const [type] <target>
typedef struct IRConstant {
	int64_t  target;
	uint32_t ID;
} IRConstant; 

// tID = neg [type] <target>
typedef struct IRNegate {
	IRInst* target;
	uint32_t ID;
} IRNegate;

// tID = cast [type] <target>
typedef struct IRCastType {
	IRInst* target;
	Type* type;
	uint32_t ID;
} IRCastType;

Vector* GenIR(Vector* stats, Vector* symtab);
void PrintIR(Vector* IR);

#endif
