#include "sema.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int counter = 1;
int GetCounter() {
	int ctr = counter++;
	return ctr;
}

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

void OpImm(IRInst* inst, int idx, int imm) {
	inst->operands[idx].type = OP_IMM;
	inst->operands[idx].imm = imm;
}

void OpResource(IRInst* inst, int idx, int id) {
	inst->operands[idx].type = OP_RESOURCE;
	inst->operands[idx].resource = id;
}

IRInst* MakeIRInst(enum IRInstruction inst, uint32_t flags) {
	IRInst* ret = malloc(sizeof(IRInst));
	if (!ret)
		return NULL;

	ret->inst = inst;
	ret->flags = flags;
	ret->id = 0;
	return ret;
}

enum IRInstruction BOT2IR(enum BinaryOpType ty) {
	switch(ty) {
		case BOT_ADD: return IR_ADD;
		case BOT_SUB: return IR_SUB;
		case BOT_MUL: return IR_MUL;
		case BOT_DIV: return IR_DIV;
		case BOT_MOD: return IR_MODULUS;
		default: {
			printf("BOT2IR(): Unknown value of ty = %d\n", ty);
			return IR_MAX;
		}

	}
}

int GenIRExpr(Vector* IR, Expr* expr, Vector* symtab) {
	switch (expr->type) {
		case ET_INT_LITERAL: {
			IRInst* ret = MakeIRInst(IR_NEW, 0);
			OpImm(ret, 0, 4);
			ret->id = GetCounter();
			Append(IR, ret);

			IRInst* store = MakeIRInst(IR_STORE, 0);
			OpResource(store, 0, ret->id);
			OpImm(store, 1, 4);
			OpImm(store, 2, expr->literal);
			Append(IR, store);

			return ret->id;
		}

		case ET_BINARY_OP: {
			BinaryOp* binop = expr->binop;
			int left = GenIRExpr(IR, binop->left, symtab);
			int right = GenIRExpr(IR, binop->right, symtab);

			enum IRInstruction inst = BOT2IR(expr->binop->type);
			IRInst* ret = MakeIRInst(inst, 0);
			OpResource(ret, 0, left);
			OpResource(ret, 1, right);
			ret->id = GetCounter();
			Append(IR, ret);

			return ret->id;
		}

		case ET_IDENT: {
			printf("GenIRExpr(): codegen is not implemented for ET_IDENT\n");
			return 0;
		}
	}

	return 0; 
}

int GenIRVarDecl(Vector* IR, VarDecl* vardecl, Vector* symtab) {
	printf("GenIRVarDecl(): Codegen is not implemented for "
			"variable declarations\n");
	return 0; 
}

Vector* GenIR(Vector* stats, Vector* symtab) {
	if (!VectorLength(stats))
		return NULL;

	Vector* IR = NewVector();

	for (uint32_t idx = 0; idx < VectorLength(stats); idx++) {
		Statement* st = Get(stats, idx);
		switch (st->type) {
			case ST_EXPR: {
				//if (GenIRExpr(IR, st->expr, symtab))
				//	return NULL;
				GenIRExpr(IR, st->expr, symtab);
				break;
			}

			case ST_VARDECL: {
				if (GenIRVarDecl(IR, st->vardecl, symtab))
					return NULL;
			
				break;
			}

			default: {
				printf("GenIR(): IR Generation is not implemented "
					"for st->type = %d\n", st->type);
				return NULL;
			}
		}
	}

	return IR;
}

const char* IR2S[] = {
	"new", "store", "load", "add",
	"sub", "mul", "div", "mod"
};

const int lenmap[] = {
	1, 3, 0, 2, 2, 2, 2, 2
};

void PrintIR(Vector* IR) {
	for (uint32_t idx = 0; idx < VectorLength(IR); idx++) {
		IRInst* inst = Get(IR, idx);
		if (inst->id)
			printf("t%d = ", inst->id);

		printf("%s ", IR2S[inst->inst]);
		for (int i = 0; i < lenmap[inst->inst]; i++) {
			Operand* op = &inst->operands[i];
			if (op->type == OP_RESOURCE)
				printf("t%d ", op->resource);
			else
				printf("%d ", op->imm);
		}

		printf("\n");
	}
}

int main(int argc, const char** argv) {
	if (argc < 2) 
		return 1;

	Lexer lexer; 
	if (NewLexer(argv[1], &lexer))
		return 1;

	Token* token = NULL;
	Vector* stats = NewVector();

	while (1) {
		token = Peek(&lexer);
		if (token->type == TT_EOF) {
			Next(&lexer);
			break;
		}

		Statement* stat = ParseStatement(&lexer);
		if (!stat)
			break;

		Append(stats, stat);

		DumpStatement(stat);
		printf("\n");
	}

	if (!VectorLength(stats))
		return 0;

	Vector* symtab = SemanticAnalyse(&lexer, stats);
	if (!symtab)
		return 2;

	Vector* ir = GenIR(stats, symtab);
	if (!ir) {
		printf("Internal Error: IR Generation failed\n");
		return 3;
	}

	printf("IR Instructions = %u\n", VectorLength(ir));
	PrintIR(ir);

	DeleteLexer(&lexer);
	return 0;
}
