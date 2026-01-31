/* Reference: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static void ParserError(Lexer* lexer, Token* token, const char* msg) {
	printf("%s:%u:%u Error: %s\n", lexer->file, token->line, 
			token->pos, msg);
}

int Expect(Lexer* lexer, TokenType type, const char* msg) {
	Token* token = Next(lexer);
	if (token->type == type)
		return 1;

	ParserError(lexer, token, msg);
	return 0;
}

static int IsOperator(TokenType type) {
	switch (type) {
		case TT_PLUS: case TT_MINUS: case TT_ASTERISK: 
		case TT_SLASH: case TT_EQUALS: return 1;
		default: return 0;
	}
}

static uint8_t infix_binding_power[] = {
	(9 << 4) | 10, // + , -
	(11 << 4) | 12, // * , /
	(1 << 4) | 2, // =
};

static int OpToInfixIndex(TokenType type) {
	switch (type) {
		case TT_PLUS: 
		case TT_MINUS: return 0;
		case TT_ASTERISK: 
		case TT_SLASH: return 1;
		case TT_EQUALS: return 2;
		default: return -1;
	}
}

static const char* BOT_TO_STRING[] = {
	"+", "-", "*", "/", "="
};

static enum BinaryOpType OpToBinop(TokenType type) {
	switch (type) {
		case TT_PLUS: return BOT_ADD;
		case TT_MINUS: return BOT_SUB;
		case TT_ASTERISK: return BOT_MUL;
		case TT_SLASH: return BOT_DIV;
		case TT_EQUALS: return BOT_EQUALS;
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

static Expr* PrattParseExpr(Lexer* lexer, Expr* expr, int minbp) {
	Expr* lhs = NULL;
	if (!expr) {
		Token* tlhs = Next(lexer);
		if (tlhs->type != TT_NUMBER && tlhs->type != TT_IDENT) {
			ParserError(lexer, tlhs, "Expected literal or identifier here");
			return NULL;
		}

		lhs = makeExpr(ET_LITERAL);
		lhs->literal = tlhs;
	}  else { 
		lhs = expr;
	}
	
	while (1) {
		Token* op = Peek(lexer);
		if (op->type == TT_SEMICOLON || op->type == TT_EOF)
			break;

		if (!IsOperator(op->type)) {
			// ParserError(lexer, op, "Expected operator here");
			break;
		}

		uint8_t bp = infix_binding_power[OpToInfixIndex(op->type)];
		uint8_t l_bp = bp >> 4, r_bp = bp & 0xf;
		
		if (l_bp < minbp)
			break;

		Next(lexer);
		Expr* rhs = PrattParseExpr(lexer, NULL, r_bp);

		Expr* tmp = makeExpr(ET_BINARY_OP);
		tmp->binop = malloc(sizeof(BinaryOp));
		tmp->binop->type = OpToBinop(op->type);
		tmp->binop->left = lhs;
		tmp->binop->right = rhs;

		lhs = tmp;
	}

	return lhs;
}

static Expr* ParseExpression(Lexer* lexer, Expr* expr) {
	return PrattParseExpr(lexer, expr, 0);
}

int ParseVarDecl(Lexer* lexer, Statement* stat) {
	stat->type = ST_VARDECL;

	Token* token = Peek(lexer);
	if (token->type != TT_IDENT) {
		ParserError(lexer, token, "Expected an identifier");
		return 1;
	}

	Next(lexer);

	Token* op = Peek(lexer);
	Token* ty = NULL;

	if (op->type == TT_COLON) {
		Next(lexer);
		ty = Next(lexer);
		if (ty->type != TT_IDENT) {
			ParserError(lexer, ty, "Expected a type name here");
			return 1;
		}
	}
	else {
		if (op->type != TT_EQUALS && op->type != TT_SEMICOLON) {
			ParserError(lexer, op, "Expected '=' or ';' here");
			return 1;
		}
	}

	Expr* expr = (op->type == TT_SEMICOLON) ? NULL : makeExpr(ET_LITERAL);
	if (!expr)
		goto no_init;

	expr->literal = token;

	expr = ParseExpression(lexer, expr);
	if (!expr)
		return 1;

no_init:
	stat->vardecl = malloc(sizeof(VarDecl));
	stat->vardecl->ident = token;
	stat->vardecl->init = expr;
	stat->vardecl->type = ty;
	return 0;
}

/* 
 * statement ::= expr ';' | 
 * "let" ident (':' type ) '=' expr ';'
 */

Statement* ParseStatement(Lexer* lexer) {
	Token* token = Peek(lexer);

	if (token->type == TT_EOF) {
		Next(lexer);
		return NULL;
	}

	Statement* stat = malloc(sizeof(Statement));
	if (!stat)
		return NULL;

	switch (token->type) {
		case TT_LET: {
			Next(lexer);
			if (ParseVarDecl(lexer, stat))
				return NULL;
			break;
		}

		default: {
			Expr* expr = ParseExpression(lexer, NULL);
			if (!expr) 
				return NULL;
			stat->type = ST_EXPR;
			stat->expr = expr;
		}
	}

	if (!Expect(lexer, TT_SEMICOLON, "Expected semicolon here"))
		return NULL;

	return stat;
}

void DumpStatement(Lexer* lexer, Statement* stat) {
	if (stat->type == ST_EXPR)
		DumpExpr(lexer, stat->expr);
	else {
		if (stat->vardecl->init)
			DumpExpr(lexer, stat->vardecl->init);
		else {
			DumpToken(lexer, stat->vardecl->ident);
			printf("= ");
			printf("no-init ");
		}

		if (stat->vardecl->type)
			DumpToken(lexer, stat->vardecl->type);
	}
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

