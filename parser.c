/* Reference: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html */

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

static void ParserError(Lexer* lexer, Token* token, const char* msg) {
	printf("%s:%u:%u Error: %s\n", lexer->file, token->line, 
			token->pos, msg);
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

static Expr* PrattParseExpr(Lexer* lexer, int minbp) {
	Token* tlhs = Next(lexer);
	if (tlhs->type != TT_NUMBER && tlhs->type != TT_IDENT) {
		ParserError(lexer, tlhs, "Expected literal or identifier here");
		return NULL;
	}

	Expr* lhs = makeExpr(ET_LITERAL);
	lhs->literal = tlhs;

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

int ParseVarDecl(Lexer* lexer, Statement* stat) {
	Token* token = Peek(lexer);
	Expr* expr = ParseExpression(lexer);
	if (!expr)
		return 1;

	stat->vardecl = malloc(sizeof(VarDecl));
	if (expr->type == ET_LITERAL) {
		stat->vardecl->ident = expr->literal;
		stat->vardecl->init = NULL;
	} else {
		if (expr->binop->type != BOT_EQUALS) {
			ParserError(lexer, token, "Expected an assignment expression here");
			return 1;
		}

		if (expr->binop->left->type != ET_LITERAL) {
			ParserError(lexer, token, "Expected an identifier here");
			return 1;

		}

		stat->vardecl->ident = expr->binop->left->literal;
		stat->vardecl->init = expr->binop->right;
	}
		
	stat->type = ST_VARDECL;
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
			Expr* expr = ParseExpression(lexer);
			if (!expr) 
				return NULL;
			stat->type = ST_EXPR;
			stat->expr = expr;
		}

	}

	token = Next(lexer);
	if (token->type != TT_SEMICOLON) {
		ParserError(lexer, token, "Expected semicolon here");
		return NULL;
	}

	return stat;
}

void DumpStatement(Lexer* lexer, Statement* stat) {
	if (stat->type == ST_EXPR)
		DumpExpr(lexer, stat->expr);
	else {
		DumpToken(lexer, stat->vardecl->ident);
		printf("= ");
		if (stat->vardecl->init)
			DumpExpr(lexer, stat->vardecl->init);
		else
			printf("no-init ");
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

