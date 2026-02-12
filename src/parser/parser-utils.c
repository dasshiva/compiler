#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser-utils.h"

int IsPrefixOperator(TokenType type) {
	return (type == TT_PLUS) || (type == TT_MINUS);
}

int IsOperator(TokenType type) {
	switch (type) {
		case TT_PLUS: case TT_MINUS: case TT_ASTERISK: 
		case TT_SLASH: case TT_EQUALS: case TT_PERCENT: return 1;
		default: return 0;
	}
}

enum UnaryOpType OpToUnop(TokenType type) {
	switch (type) {
		case TT_PLUS: return UOT_ADD;
		case TT_MINUS: return UOT_MINUS;
		default: return UOT_MAX; // unreachable
	}
}

enum BinaryOpType OpToBinop(TokenType type) {
	switch (type) {
		case TT_PLUS: return BOT_ADD;
		case TT_MINUS: return BOT_SUB;
		case TT_ASTERISK: return BOT_MUL;
		case TT_SLASH: return BOT_DIV;
		case TT_PERCENT: return BOT_MOD;
		case TT_EQUALS: return BOT_EQUALS;
	}

	return BOT_MAX; // unreachable
}

Expr* makeExpr(enum ExprType type) {
	Expr* expr = malloc(sizeof(Expr));
	if (!expr)
		return NULL;

	expr->type = type;
	return expr;
}

void DumpStatement(Statement* stat) {
	if (stat->type == ST_EXPR)
		DumpExpr(stat->expr);
	else {
		if (stat->vardecl->init)
			DumpExpr(stat->vardecl->init);
		else {
			printf("%s ", stat->vardecl->ident);
			printf("= ");
			printf("no-init ");
		}

		if (stat->vardecl->type)
			printf("%s ", stat->vardecl->type);
	}
}

static const char* UOT_TO_STRING[] = { "u+", "u-" };
static const char* BOT_TO_STRING[] = {
	"+", "-", "*", "/", "=", "%"
};

void DumpExpr(Expr* expr) {
	if (expr->type == ET_INT_LITERAL)
		printf("%d ", expr->literal->number);
	else if (expr->type == ET_IDENT)
		printf("%s ", expr->ident);
	else if (expr->type == ET_UNARY_OP) {
		DumpExpr(expr->unop->operand);
		printf("%s ", UOT_TO_STRING[expr->unop->type]);
	}
	else if (expr->type == ET_BINARY_OP) {
		DumpExpr(expr->binop->left);
		DumpExpr(expr->binop->right);
		printf("%s ", BOT_TO_STRING[expr->binop->type]); 
	}
}

void ParserError(Lexer* lexer, Token* token, const char* msg) {
	printf("%s:%u:%u\nError: %s\n", lexer->file, token->line, 
			token->pos, msg);
	
	LineExtent* le = Get(lexer->extent, token->line - 1);
	if (le == INVALID_INDEX)
		return;

	char* source = lexer->source + le->offset;
	// If line length is not known, assume it ends where the token ends
	if (!le->length) 
		le->length = token->offset - le->offset + token->length;

	char* line = malloc(sizeof(char) * (le->length + 1));
	memcpy(line, source, le->length);
	line[le->length] = '\0';

	printf("%s\n", line);
	for (uint32_t i = 0; i < token->pos - 1; i++) { printf(" "); }
	printf("^\n");
	free(line);
}

int Expect(Lexer* lexer, TokenType type, const char* msg) {
	Token* token = Next(lexer);
	if (token->type == type)
		return 1;

	ParserError(lexer, token, msg);
	return 0;
}

int ToDecimal(int digit, int base) {
	if (digit < 0)
		return -1;

	if (base == 2) {
		if (digit < 2)
			return digit;
		else
			return -1;
	}

	else if (base == 8) {
		if (digit < 8)
			return digit;
		else
			return -1;
	}

	else if (base == 10) {
		if (digit < 10)
			return digit;
		else
			return -1;
	}

	else { // base = 16
		switch (digit) {
			case 0: case 1: case 2: case 3: case 4: case 5: case 6:
			case 7: case 8: case 9: return digit;
			case 'A' - '0': case 'a' - '0': return 10;
			case 'B' - '0': case 'b' - '0': return 11;
			case 'C' - '0': case 'c' - '0': return 12;
			case 'D' - '0': case 'd' - '0': return 13;
			case 'E' - '0': case 'e' - '0': return 14;
			case 'F' - '0': case 'f' - '0': return 15;
			default: return -1;
		}
	}
}
