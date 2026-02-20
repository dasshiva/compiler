/* Reference: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser-utils.h"
#include "parse-expr.h"

Type* ParseLiteralSuffix(char* source, uint32_t len) {
	for (int i = 0; i < len_builtins; i++) {
		Type* ty = BUILTIN_TYPES[i];
		if (strncmp(source, ty->name, len) == 0)
			return ty;
	}

	return NULL;
}

int ParseIntLiteral(Lexer* lexer, Token* token, Expr* expr) {
	char* source = lexer->source + token->offset;
	int base = 10;

	expr->literal = malloc(sizeof(IntLiteral));
	
	if (*source == '0') { 
		if (token->length == 1) {
			expr->literal->type = I32();
			expr->literal->number = 0;
			return 1;
		}

		source++;
		switch (*source) {
			case 'b': case 'B' : base = 2; token->length -= 2;  break;
			case 'x': case 'X' : base = 16; token->length -= 2; break;
			default: {
				if (isdigit(*source)) {
					// move source back, so that the later source++ puts us
					// back in the same place
					source--;
					base = 8;
					token->length -= 1;
				}
				else {
					ParserError(lexer, token, "Invalid base specifier"
							" in integer literal");
					return 0;
				}
			}
		}
		source++;
	}

	int64_t literal = 0;
	Type* type = NULL;

	for (uint32_t idx = 0; idx < token->length; idx++) {
		int d = ToDecimal(*source - '0', base);
		if (d == -1) {
			uint32_t diff = token->length - idx;
			if (diff > 1 && diff < 4) {
				type = ParseLiteralSuffix(source, diff);
				if (type) 
					break;
			}

			ParserError(lexer, token, "Non-digit present in number literal");
			return 0;
		}

		literal = literal * base + d;
		source++;
	}

	expr->literal->type = (type) ? type : I32();
	expr->literal->number = literal;
	return 1;
}

int ParseIdent(Lexer* lexer, Token* token, Expr* expr) {
	expr->ident = malloc(sizeof(char) * (token->length + 1));
	if (!expr->ident)
		return 0;

	memcpy(expr->ident, lexer->source + token->offset, token->length);
	expr->ident[token->length] = '\0';
	return 1;
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

	stat->loc.line = token->line;
	stat->loc.pos = token->pos;

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

		op = Peek(lexer); // move to the next token
	}
	
	if (op->type != TT_EQUALS && op->type != TT_SEMICOLON) {
		ParserError(lexer, op, "Expected '=' or ';' here");
		return 1;
	}

	Expr* expr = (op->type == TT_SEMICOLON) ? NULL : makeExpr(ET_IDENT);
	if (!expr) {
		if (!ty) {
			ParserError(lexer, token, "Variables without an initializer "
				"must have a type provided to them");
			return 1;
		}

		goto no_init;
	}

	if (!ParseIdent(lexer, token, expr))
		return 1;

	expr = ParseExpression(lexer, expr);
	if (!expr)
		return 1;

no_init:
	stat->vardecl = malloc(sizeof(VarDecl));
	stat->vardecl->init = expr;

	// Trick ParseIdent into parsing these for us
	Expr tmp;
	ParseIdent(lexer, token, &tmp);
	stat->vardecl->ident = tmp.ident;
	if (ty) {
		ParseIdent(lexer, ty, &tmp);
		stat->vardecl->type = tmp.ident;
		stat->vardecl->loc_type.line = ty->line;
		stat->vardecl->loc_type.pos = ty->pos;
	} else
		stat->vardecl->type = NULL;

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
			stat->loc.line = token->line;
			stat->loc.pos = token->pos;

			Expr* expr = ParseExpression(lexer, NULL);
			if (!expr) 
				return NULL;
			stat->type = ST_EXPR;
			stat->expr = expr;
		}
	}

	if (!Expect(lexer, TT_SEMICOLON, "Expected ';' here"))
		return NULL;

	return stat;
}


