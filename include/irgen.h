#ifndef __IRGEN_H__
#define __IRGEN_H__

#include "sema.h"
#include <stdint.h>

enum IRInstruction {
	IR_NEW, IR_STORE, IR_LOAD, IR_ADD, IR_SUB,
	IR_MUL, IR_DIV, IR_MODULUS, IR_CONST, IR_COPY,
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
	Instruction* operands;
} IRInst;

// t%ID = {binary-op} [type] <left>, <right>
typedef struct IRBinaryOp {
	IRType*  type;
	uint32_t left;
	uint32_t right;
	uint32_t ID;
} IRBinaryOp; 

// t%ID = copy <target>
typedef struct IRCopyValue {
	uint32_t ID;
	uint32_t target;
} IRCopyValue;

// tID = alloc [type]
typedef struct IRAllocate {
	IRType* type;
} IRAllocate;

// tID = const [type] <target>
typedef struct IRConstant {
	IRType*  type;
	int64_t  target;
	uint32_t ID;
} IRConstant; 

// store <dest>, <source>
typedef struct IRStoreMemory {
	uint32_t dest;
	uint32_t source;
} IRStoreMemory;

// tID = load <dest>, <source>
typedef struct IRLoad {
	uint32_t dest;
	uint32_t source;
	uint32_t ID;
} IRLoadMemory;

Vector* GenIR(Vector* stats, Vector* symtab);
void PrintIR(Vector* IR);

#endif
