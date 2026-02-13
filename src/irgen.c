#include "irgenhelpers.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

typedef struct ResourceMetadata {
	Type* type;
	IRInst* id;
} RMD;

#define IRGEN_EVALUATE_SUCCESS ((void*)1)

static IRInst* GenIRExprRecurse(Vector* IR, Expr* expr, 
		Vector* symtab, int level) {

	switch (expr->type) {
		case ET_INT_LITERAL: {
			IRInst* ret = IRConst(IR, expr->literal->type, expr->literal->number);
			if (!level)
				goto clean_exit;

			return ret;
		}

		case ET_UNARY_OP: {
			UnaryOp* unop = expr->unop;
			IRInst* operand = GenIRExprRecurse(IR, unop->operand,
					symtab, level + 1);

			IRInst* ret = operand;

			switch (unop->type) {
				case UOT_ADD: break; // A unary add is effectively a nop
				case UOT_MINUS: {
					ret = IRNeg(IR, operand, NULL); break;
				}
			}

			if (!level)
				goto clean_exit;

			return ret;
		}

		case ET_BINARY_OP: {
			BinaryOp* binop = expr->binop;
			IRInst* left = GenIRExprRecurse(IR, binop->left, 
					symtab, level + 1);
			IRInst* right = GenIRExprRecurse(IR, binop->right, 
					symtab, level + 1);

			IRInst* ret = NULL;
			switch (binop->type) {
				case BOT_ADD: ret = IRAdd(IR, left, right, NULL); break;
				case BOT_SUB: ret = IRSub(IR, left, right, NULL); break;
				case BOT_MUL: ret = IRMul(IR, left, right, NULL); break;
				case BOT_DIV: ret = IRDiv(IR, left, right, NULL); break;
				case BOT_MOD: ret = IRMod(IR, left, right, NULL); break;
				case BOT_EQUALS: {
					RMD* saved = NULL;
					for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
						Symbol* var = Get(symtab, idx);
						if (var->type != TYPE_VARIABLE)
							continue;

						RMD* rmd = var->data;
						if (left == rmd->id) {
							saved = rmd;
							break;
						}
					}

					saved->id = right;
					ret = saved->id;
					break;
				}
				default: {
					printf("GenIRExprRecurse(); IRGen not implemented for "
						"expr->binop->type = %d\n", expr->binop->type);
					return 0;
				}
			}

			if (!level)
				goto clean_exit;

			return ret;
		}

		case ET_IDENT: {
			RMD* rmd = GetVariable(symtab, expr->ident)->data;
			if (!level)
				goto clean_exit;

			return rmd->id;
		}

		default: {
			printf("IRGenExprRecurse(): expr->type %d does not "
				"have IRGen implemented for it\n", expr->type);
			return 0;
		}
	}

clean_exit:
	return IRGEN_EVALUATE_SUCCESS;
}

static int GenIRExpr(Vector* IR, Expr* expr, Vector* symtab) {
	return GenIRExprRecurse(IR, expr, symtab, 0) == 
		IRGEN_EVALUATE_SUCCESS;
}

static int GenIRVarDecl(Vector* IR, VarDecl* vardecl, 
		Vector* symtab) {

	RMD* rmd = malloc(sizeof(RMD));
	rmd->type = GetType(symtab, vardecl->type);
	rmd->id = NULL;

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

	for (uint32_t idx = 0; idx < VectorLength(stats); idx++) {
		Statement* st = Get(stats, idx);
		switch (st->type) {
			case ST_EXPR: {
				if (!GenIRExpr(IR, st->expr, symtab))
					return NULL;
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
	"sub", "mul", "div", "mod", "constant", "neg"
};

void PrintIR(Vector* IR) {
	/* for (uint32_t idx = 0; idx < VectorLength(IR); idx++) {
		IRInst* inst = Get(IR, idx);
		switch (inst->code) {
			case IR_ADD: 
			case IR_SUB:
			case IR_MUL:
			case IR_DIV:
			case IR_MODULUS: {
				IRBinaryOp* op = inst->operands;
				printf("t%u = ", op->ID);
				printf("%s ", IR2S[inst->code]);
				if (op->type)
					printf("%s ", op->type->name);
				printf("t%u, ", op->left);
				printf("t%u\n", op->right);
				break;
			}

			case IR_CONST: {
				IRConstant* cts = inst->operands;
				printf("t%u = ", cts->ID);
				printf("%s ", IR2S[inst->code]);
				if (cts->type)
					printf("%s ", cts->type->name);

				printf("%ld\n", cts->target);
				break;
			}

			case IR_NEG: {
				IRNegate* neg = inst->operands;
				printf("t%u = ", neg->ID);
				printf("%s ", IR2S[inst->code]);
				if (neg->type)
					printf("%s ", neg->type->name);

				printf("t%ld\n", neg->target);
				break;
			}

			default: {
				printf("PrintIR(): Printing IR is not implemented "
					"for inst->code = %d\n", inst->code);
				break;
			}
		}
	} */
}

