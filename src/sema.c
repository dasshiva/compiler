#include "sema.h"
#include "parser-utils.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct Error {
	Location* loc;
	char message[400];
} Error;


static void MakeError(Error* err, Location* loc, const char* msg, ...) {
	va_list ap;
	va_start(ap, msg);

	err->loc = loc;
	vsnprintf(err->message, 400, msg, ap);
	va_end(ap);
}

static void SemaError(Lexer* lexer, Error* err) {
	printf("%s:%u:%u\nError: %s\n", lexer->file, err->loc->line, 
			err->loc->pos, err->message);

	LineExtent* le = Get(lexer->extent, err->loc->line - 1);
	char* source = lexer->source + le->offset;
	char* line = malloc(sizeof(char) * (le->length + 1));
	memcpy(line, source, le->length);
	line[le->length] = '\0';

	printf("%s\n", line);
	for (uint32_t i = 0; i < err->loc->pos - 1; i++) { printf(" "); }
	printf("^\n");
	free(line);
}

Symbol* MakeSymbol(const char* name, enum SymbolType type, uint32_t flags) {
	Symbol* ret = malloc(sizeof(Symbol));
	if (!ret)
		return NULL;

	ret->name = name;
	ret->type = type;
	ret->flags = flags;

	return ret;
}

Type* GetType(Vector* symtab, const char* name) {
	for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
		Symbol* sym = Get(symtab, idx);
		if (sym->type != TYPE_TYPEDEF)
			continue;

		if (strcmp(name, sym->name) == 0)
			return sym->utype;
	}

	return NULL;
}

Symbol* GetVariable(Vector* symtab, const char* name) {
	for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
		Symbol* sym = Get(symtab, idx);
		if (sym->type != TYPE_VARIABLE)
			continue;

		if (strcmp(name, sym->name) == 0)
			return sym;
	}

	return NULL;
}

static Type* ConvertTagToType(const int* tag, Vector* symtab) {
	Type* ret = NULL;

	for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
		Symbol* sym = Get(symtab, idx);
		if (sym->type == TYPE_TYPEDEF) {
			if (sym->utype->tag == *tag) {
				ret = sym->utype;
				break;
			}
		}
	}

	return ret;
}

static const int* ConvertTypeToTag(const Type* type) {
	return &type->tag;
}

static int SemaExprEvaluate(Vector* opstack, Expr* expr, 
		Vector* symtab, Error* err) {
	switch (expr->type) {
		case ET_INT_LITERAL: {
			Append(opstack, ConvertTypeToTag(expr->literal->type));
			return 1;
		}

		case ET_IDENT: {
			Symbol* sym = GetVariable(symtab, expr->ident);
			if (!sym) {
				MakeError(err, &expr->loc, "Undefined identifier %s "
					"(NOTE: you cannot use a variable in its own initializer)",
						expr->ident);
				return 0;
			}

			if (!sym->utype) {
				MakeError(err, &expr->loc, 
					"Variable %s, has not been assigned a type", expr->ident);
				return 0;

			}

			const int* type = ConvertTypeToTag(sym->utype);
			Append(opstack, type);
			return 1;
		}

		case ET_UNARY_OP: {
			int l = SemaExprEvaluate(opstack, expr->unop->operand, symtab, err);
			if (!l) 
				return 0;

			int* lhs = Pop(opstack);
			if (!TypeSupportsOp(ConvertTagToType(lhs, symtab), 
						expr->unop->type)) {
				MakeError(err, &expr->loc, "Invalid type for unary operator");
				return 0;

			}

			if (lhs == INVALID_INDEX) {
				printf("SemaExprEvaluate(): lhs == NULL\n");
				return 0;
			}

			Append(opstack, lhs);
			return 1;
		}

		case ET_CAST: {
			int l = SemaExprEvaluate(opstack, expr->cast->expr, symtab, err);
			if (!l)
				return 0;

			int *lhs = Pop(opstack);
			if (lhs == INVALID_INDEX) {
				printf("SemaExprEvaluate(): lhs == NULL\n");
				return 0;
			}

			Append(opstack, ConvertTypeToTag(expr->cast->target));
			return 1;
		}

		case ET_BINARY_OP: {
			int l = SemaExprEvaluate(opstack, expr->binop->left, symtab, err);
			int r = SemaExprEvaluate(opstack, expr->binop->right, symtab, err);
			if (!l || !r)
				return 0;

			int* rhs = Pop(opstack);
			int* lhs = Pop(opstack);
			if (lhs == INVALID_INDEX || rhs == INVALID_INDEX) {
				printf("SemaExprEvaluate(): lhs == NULL || rhs == NULL\n");
				return 0;
			}

			Type* tlhs = ConvertTagToType(lhs, symtab);
			Type* trhs = ConvertTagToType(rhs, symtab);

			if (!TypeSupportsOp(tlhs, expr->binop->type)) {
				MakeError(err, &expr->binop->left->loc, "Incompatible operand for operator of type %s", tlhs->name);
				return 0;
			}

			if (!TypeSupportsOp(trhs, expr->binop->type)) {
				MakeError(err, &expr->binop->right->loc, "Incompatible operand for operator of type %s", trhs->name);
				return 0;

			}

			if (*lhs != *rhs) {
				int l2r = (expr->binop->type == OP_BINARY_EQUALS) 
					? 0 : TypesCompatible(tlhs, trhs);
				int r2l = TypesCompatible(trhs, tlhs);
				Expr* target = NULL;

				if (l2r) {
					Expr* saved = expr->binop->left;
					target = makeExpr(ET_CAST);
					target->cast = malloc(sizeof(Cast));
					target->cast->target = trhs;
					target->cast->expr = saved;
					target->loc = saved->loc;
					expr->binop->left = target;
					*lhs = *rhs;
				}

				else if (r2l) {
					Expr* saved = expr->binop->right;
					target = makeExpr(ET_CAST);
					target->cast = malloc(sizeof(Cast));
					target->cast->target = tlhs;
					target->cast->expr = saved;
					target->loc = saved->loc;
					expr->binop->right = target;
				}

				else {
					MakeError(err, &expr->loc, "Mismatching types for operator (%s and %s)", tlhs->name, trhs->name);
					return 0;
				}
			}

			Append(opstack, lhs);
			return 1;
		}

		default: {
			printf("Sema has not been implemented for expr->type = %d\n",
					expr->type);
			return 0;
		}
	}
}

static Type* SemaExpression(Expr* expr, Vector* symtab, Error* err) {
	Vector* opstack = NewVector();
	if (!SemaExprEvaluate(opstack, expr, symtab, err))
		return NULL;
	
	if (VectorLength(opstack) != 1) {
		printf("SemaExpression(): len(opstack) != 1\n");
		return NULL;
	}

	return ConvertTagToType(Pop(opstack), symtab);
}


static int SemaVarDecl(Statement* stat, Vector* symtab, Error* err) {
	VarDecl* vardecl = stat->vardecl;
	Type* ty = NULL;

	if (vardecl->type) {
		ty = GetType(symtab, vardecl->type);
		if (!ty) {
			MakeError(err, &vardecl->loc_type, "Unknown type name %s", vardecl->type);
			return 0;
		}
	}

	Symbol* sym = MakeSymbol(vardecl->ident, TYPE_VARIABLE, 0);
	Append(symtab, sym);
	sym->utype = (ty) ? ty : NULL;

	if (vardecl->init) {
		Type* type = SemaExpression(vardecl->init, symtab, err);
		if (!type)
			return 0;

		if (ty) {
			if (ty != type) {
				MakeError(err, &vardecl->loc, "Mismatch between types of "
					"variable %s and initializer expression", vardecl->ident);
				return 0;
			}
			else
				sym->utype = type;
		}
		else
			sym->utype = type;
	}

	Append(symtab, sym);
	return 1;
}

static int SemaStatement(Statement* stat, Vector* symtab, Error* err) {
	switch (stat->type) {
		case ST_VARDECL: return SemaVarDecl(stat, symtab, err);
		case ST_EXPR: return SemaExpression(stat->expr, symtab, err) != NULL;
		default: {
			printf("SemaStatement(): Unknown statement type\n");
			break;
		}
	}

	return 0;
}


Vector* SemanticAnalyse(Lexer* lexer, Vector* statements) {
	if (!VectorLength(statements))
		return NULL;

	Vector* symtab = NewVector();
	for (int i = 0; i < len_builtins; i++) {
		Symbol* sym = MakeSymbol(BUILTIN_TYPES[i]->name, TYPE_TYPEDEF, 0);
		sym->utype = BUILTIN_TYPES[i];
		Append(symtab, sym);
	}

	int error = 0;
	for (uint32_t i = 0; i < VectorLength(statements); i++) {
		Error err;
		if (!SemaStatement(Get(statements, i), symtab, &err)) {
			SemaError(lexer, &err);
			error = 1;
		}
	}

	return (error) ? NULL : symtab;
}
