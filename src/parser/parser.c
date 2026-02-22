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
		return 0;
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
			return 0;
		}

		op = Peek(lexer); // move to the next token
	}
	
	if (op->type != TT_EQUALS && op->type != TT_SEMICOLON) {
		ParserError(lexer, op, "Expected '=' or ';' here");
		return 0;
	}

	Expr* expr = (op->type == TT_SEMICOLON) ? NULL : makeExpr(ET_IDENT);
	if (!expr) {
		if (!ty) {
			ParserError(lexer, token, "Variables without an initializer "
				"must have a type provided to them");
			return 0;
		}

		goto no_init;
	}

	if (!ParseIdent(lexer, token, expr))
		return 0;

	expr = ParseExpression(lexer, expr);
	if (!expr)
		return 0;

no_init:
	stat->vardecl = malloc(sizeof(VarDecl));
	stat->vardecl->init = expr;
	stat->vardecl->loc.line = token->line;
	stat->vardecl->loc.pos = token->pos;

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

	return 1;
}

/*
 * function ::= "function" name '(' args (',' args) ')' 
 	("->" type) '{' (statement) '}'
 * args ::= name ':' type
 *
 */
int ParseFunction(Lexer* lexer, Statement* stat) {
	Token* fname = Next(lexer);
	if (fname->type != TT_IDENT) {
		ParserError(lexer, fname, "Expected function name here");
		return 0;
	}

	stat->type = ST_FUNCTION;
	stat->func = malloc(sizeof(Function));
	if (!stat->func)
		return 0;

	Expr tmp;
	ParseIdent(lexer, fname, &tmp);
	stat->func->name = tmp.ident;

	if (!Expect(lexer, TT_LPAREN, "Expected '(' here"))
		return 0;

	stat->func->args = NewVector();
	
	int more_params = 0;
	while (1) {
		Token* token = Peek(lexer);
		if (token->type == TT_RPAREN) {
			if (more_params) 
				goto parse_next_param;

			Next(lexer);
			break;
		}

parse_next_param:

		if (token->type != TT_IDENT) {
			ParserError(lexer, token, "Expected name of parameter");
			return 0;
		}

		FuncArgs* fnargs = malloc(sizeof(FuncArgs));
		if (!fnargs)
			return 0;

		ParseIdent(lexer, token, &tmp);
		fnargs->name = tmp.ident;
		Next(lexer);

		if (!Expect(lexer, TT_COLON, "Expected ':' between name "
					"of parameter and type "))
			return 0;

		token = Next(lexer);
		if (token->type != TT_IDENT) {
			ParserError(lexer, token, "Expected parameter type");
			return 0;
		}

		ParseIdent(lexer, token, &tmp);
		fnargs->type = tmp.ident;
		Append(stat->func->args, fnargs);

		more_params = 0;

		token = Peek(lexer);
		if (token->type == TT_RPAREN) {
			Next(lexer);
			break;
		}

		if (token->type == TT_COMMA) {
			Next(lexer);
			more_params = 1;
			continue;
		} 
		else {
			ParserError(lexer, token, "Expected ',' between parameters");
			return 0;
		}
	}

parse_return_type:

	Token* ret = Peek(lexer);
	if (ret->type == TT_RETURNS) {
		Next(lexer);
		ret = Next(lexer);
		if (ret->type != TT_IDENT) {
			ParserError(lexer, ret, "Expected return type after '->'");
			return 0;
		}

		ParseIdent(lexer, ret, &tmp);
		stat->func->rtype = tmp.ident;
	}

	else if (ret->type == TT_LCURLY) {
		stat->func->rtype = NULL;
		Next(lexer);
		goto parse_statements;
	}

	else {
		ParserError(lexer, ret, "Expected '->' or '{' here");
		return 0;
	}

	if (stat->func->rtype) {
		if (!Expect(lexer, TT_LCURLY, "Expected '{' here"))
			return 0;
	}

parse_statements:

	stat->func->statements = NewVector();
	while (1) {
		Token* next = Peek(lexer);
		if (next->type == TT_RCURLY) {
			Next(lexer);
			break;
		}

		if (next->type == TT_EOF) {
			ParserError(lexer, next, "Expected '}' to terminate function");
			return 0;
		}

		Statement* funcstat = ParseStatement(lexer);
		if (!funcstat)
			return 0;

		Append(stat->func->statements, funcstat);
	}

	return 1;
}

/* 
 * statement ::= expr  
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

	int delim_handled = 0;

	switch (token->type) {
		case TT_LET: {
			Next(lexer);
			if (!ParseVarDecl(lexer, stat))
				return NULL;
			break;
		}

		case TT_FUNCTION: {
			Next(lexer);
			if (!ParseFunction(lexer, stat))
				return NULL;

			delim_handled = 1;
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

	if (!delim_handled) {
		if (!Expect(lexer, TT_SEMICOLON, "Expected ';' here"))
			return NULL;
	}

	return stat;
}


