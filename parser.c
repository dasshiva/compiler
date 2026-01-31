/* Reference: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static int IsOperator(TokenType type) {
	switch (type) {
		case TT_PLUS: case TT_MINUS: case TT_ASTERISK: case TT_SLASH: return 1;
		default: return 0;
	}
}

static uint8_t infix_binding_power[] = {
	(1 << 4) | 2,
	(3 << 4) | 4
};

static int OpToInfixIndex(TokenType type) {
	switch (type) {
		case TT_PLUS: 
		case TT_MINUS: return 0;
		case TT_ASTERISK: 
		case TT_SLASH: return 1;
		default: return -1;
	}
}

static const char* BOT_TO_STRING[] = {
	"+", "-", "*", "/"
};

static enum BinaryOpType OpToBinop(TokenType type) {
	switch (type) {
		case TT_PLUS: return BOT_ADD;
		case TT_MINUS: return BOT_SUB;
		case TT_ASTERISK: return BOT_MUL;
		case TT_SLASH: return BOT_DIV;
	}

	return BOT_ADD; // unreachable
}

static Expr* makeExpr(enum ExprType type) {
	Expr* expr = malloc(sizeof(Expr));
	if (!expr)
		return NULL;

	expr->type = type;
	return expr;
}

static Expr* PrattParseExpr(Lexer* lexer, int minbp) {
	Token* tlhs = Next(lexer);
	if (tlhs->type != TT_NUMBER && tlhs->type != TT_IDENT) {
		printf("%s:%u:%u Expected literal or identifier here\n", lexer->file,
				tlhs->line, tlhs->pos);
		return NULL;
	}

	Expr* lhs = makeExpr(ET_LITERAL);
	lhs->literal = tlhs;

	while (1) {
		Token* op = Peek(lexer);
		if (op->type == TT_SEMICOLON || op->type == TT_EOF)
			break;

		if (!IsOperator(op->type)) {
			printf("%s:%u:%u Expected operator here\n", lexer->file,
					op->line, op->pos);
			return NULL;
		}

		uint8_t bp = infix_binding_power[OpToInfixIndex(op->type)];
		uint8_t l_bp = bp >> 4, r_bp = bp & 0xf;
		
		if (l_bp < minbp)
			break;

		Next(lexer);
		Expr* rhs = PrattParseExpr(lexer, r_bp);

		Expr* tmp = makeExpr(ET_BINARY_OP);
		tmp->binop = malloc(sizeof(BinaryOp));
		tmp->binop->type = OpToBinop(op->type);
		tmp->binop->left = lhs;
		tmp->binop->right = rhs;

		lhs = tmp;
	}

	return lhs;
}

static Expr* ParseExpression(Lexer* lexer) {
	return PrattParseExpr(lexer, 0);
}

/* 
 * statement ::= expr ';' 
 */

Expr* ParseStatement(Lexer* lexer) {
	Token* token = Peek(lexer);

	if (token->type == TT_EOF) {
		Next(lexer);
		return NULL;
	}

	Expr* expr = ParseExpression(lexer);
	if (!expr)
		return NULL;

	token = Next(lexer);
	if (token->type != TT_SEMICOLON) {
		printf("%s:%u:%u Expected semicolon here\n", lexer->file, 
				token->line, token->pos);
		return NULL;
	}

	return expr;
}

void DumpExpr(Lexer* lexer, Expr* expr) {
	if (expr->type == ET_LITERAL)
		DumpToken(lexer, expr->literal);
	else {
		DumpExpr(lexer, expr->binop->left);
		DumpExpr(lexer, expr->binop->right);
		printf("%s ", BOT_TO_STRING[expr->binop->type]); 
	}
}

