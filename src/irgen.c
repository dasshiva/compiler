#include "irgen.h"
#include "counter.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

Counter counter;

void OpImm(IRInst* inst, int idx, int imm) {
	inst->operands[idx].type = OP_IMM;
	inst->operands[idx].imm = imm;
}

void OpResource(IRInst* inst, int idx, int id) {
	inst->operands[idx].type = OP_RESOURCE;
	inst->operands[idx].resource = id;
}

static IRInst* MakeIRInst(enum IRInstruction inst, 
		uint32_t flags) {
	IRInst* ret = malloc(sizeof(IRInst));
	if (!ret)
		return NULL;

	ret->inst = inst;
	ret->flags = flags;
	ret->id = 0;
	return ret;
}

static enum IRInstruction BOT2IR(enum BinaryOpType ty) {
	switch(ty) {
		case BOT_ADD: return IR_ADD;
		case BOT_SUB: return IR_SUB;
		case BOT_MUL: return IR_MUL;
		case BOT_DIV: return IR_DIV;
		case BOT_EQUALS: return IR_STORE;
		case BOT_MOD: return IR_MODULUS;
		default: {
			printf("BOT2IR(): Unknown value of ty = %d\n", ty);
			return IR_MAX;
		}

	}
}

typedef struct ResourceMetadata {
	Type* type;
	int id;
} RMD;

static uint64_t GenIRExprRecurse(Vector* IR, Expr* expr, 
		Vector* symtab, int level) {

	switch (expr->type) {
		case ET_INT_LITERAL: {
			IRInst* ret = MakeIRInst(IR_NEW, 0);
			OpImm(ret, 0, 4);
			ret->id = GetCounter(&counter);
			Append(IR, ret);

			IRInst* store = MakeIRInst(IR_STORE, 0);
			OpResource(store, 0, ret->id);
			OpImm(store, 1, 4);
			OpImm(store, 2, expr->literal);
			Append(IR, store);

			if (!level)
				goto clean_exit;

			return ret->id;
		}

		case ET_BINARY_OP: {
			BinaryOp* binop = expr->binop;
			int left = GenIRExprRecurse(IR, binop->left, symtab, level + 1);
			int right = GenIRExprRecurse(IR, binop->right, symtab, level + 1);

			enum IRInstruction inst = BOT2IR(expr->binop->type);
			IRInst* ret = MakeIRInst(inst, 0);
			if (inst != IR_STORE) {
				OpResource(ret, 0, left);
				OpResource(ret, 1, right);
				ret->id = GetCounter(&counter);
				Append(IR, ret);

				if (!level)
					goto clean_exit;

				return ret->id;
			} else {
				OpResource(ret, 0, left);
				Type* type = NULL;

				for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
					Symbol* symbol = Get(symtab, idx);
					if (symbol->type == TYPE_VARIABLE) {
						RMD* rmd = symbol->data;
						if (rmd->id == left) {
							type = symbol->utype;
							break;
						}
					}
				}

				if (!type) {
					printf("GenIRExpr(): BOT_EQUALS, no type for assignee\n");
					return 0;
				}

				OpImm(ret, 1, type->size);
				OpResource(ret, 2, right);
				Append(IR, ret);

				if (!level)
					goto clean_exit;

				return left;
			}

			break;
		}

		case ET_IDENT: {
			RMD* rmd = GetVariable(symtab, expr->ident)->data;
			if (!level)
				goto clean_exit;

			return rmd->id;
		}
	}

clean_exit:
	return 1;
}

static int GenIRExpr(Vector* IR, Expr* expr, Vector* symtab) {
	return GenIRExprRecurse(IR, expr, symtab, 0);
}

static int GenIRVarDecl(Vector* IR, VarDecl* vardecl, 
		Vector* symtab) {
	RMD* rmd = malloc(sizeof(RMD));
	rmd->id = GetCounter(&counter);
	rmd->type = GetType(symtab, vardecl->type);
	
	IRInst* inst = MakeIRInst(IR_NEW, 0);
	inst->id = rmd->id;
	OpImm(inst, 0, rmd->type->size);
	Append(IR, inst);

	Symbol* var = GetVariable(symtab, vardecl->ident);
	var->data = rmd;

	if (vardecl->init) 
		if (!GenIRExpr(IR, vardecl->init, symtab))
			return 0;
	
	return 1; 
}

Vector* GenIR(Vector* stats, Vector* symtab) {
	if (!VectorLength(stats))
		return NULL;

	Vector* IR = NewVector();
	ResetCounter(&counter, 1);

	for (uint32_t idx = 0; idx < VectorLength(stats); idx++) {
		Statement* st = Get(stats, idx);
		switch (st->type) {
			case ST_EXPR: {
				if (!GenIRExpr(IR, st->expr, symtab))
					return NULL;
				GenIRExpr(IR, st->expr, symtab);
				break;
			}

			case ST_VARDECL: {
				if (!GenIRVarDecl(IR, st->vardecl, symtab))
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

