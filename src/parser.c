/* Reference: https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include <ctype.h>

static void ParserError(Lexer* lexer, Token* token, const char* msg) {
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

static int IsPrefixOperator(TokenType type) {
	return (type == TT_PLUS) || (type == TT_MINUS);
}

static int IsOperator(TokenType type) {
	switch (type) {
		case TT_PLUS: case TT_MINUS: case TT_ASTERISK: 
		case TT_SLASH: case TT_EQUALS: case TT_PERCENT: return 1;
		default: return 0;
	}
}

static uint16_t infix_binding_power[] = {
	(2 << 8) | 1, // =
	(11 << 8) | 12, // + , -
	(13 << 8) | 14, // * , /, %
};

static uint16_t prefix_binding_power[] = {
	(0 << 8) | 17 // +, -
};

static int OpToPrefixIndex(TokenType type) {
	switch (type) {
		case TT_PLUS: case TT_MINUS: return 0;
		default: return -1;
	}
}

static int OpToInfixIndex(TokenType type) {
	switch (type) {
		case TT_EQUALS: return 0;
		case TT_PLUS: 
		case TT_MINUS: return 1;
		case TT_PERCENT:
		case TT_ASTERISK: 
		case TT_SLASH: return 2;
		default: return -1;
	}
}

static const char* BOT_TO_STRING[] = {
	"+", "-", "*", "/", "=", "%"
};

static enum UnaryOpType OpToUnop(TokenType type) {
	switch (type) {
		case TT_PLUS: return UOT_ADD;
		case TT_MINUS: return UOT_MINUS;
		default: return UOT_MAX; // unreachable
	}
}

static enum BinaryOpType OpToBinop(TokenType type) {
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

static Expr* makeExpr(enum ExprType type) {
	Expr* expr = malloc(sizeof(Expr));
	if (!expr)
		return NULL;

	expr->type = type;
	return expr;
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

int ParseIntLiteral(Lexer* lexer, Token* token, Expr* expr) {
	char* source = lexer->source + token->offset;
	int base = 10;

	if (*source == '0') { 
		if (token->length == 1) {
			expr->literal = 0;
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

	int literal = 0;
	for (uint32_t idx = 0; idx < token->length; idx++) {
		int d = ToDecimal(*source - '0', base);
		if (d == -1) {
			ParserError(lexer, token, "Non-digit present in number literal");
			return 1;
		}

		literal = literal * base + d;
		source++;
	}

	expr->literal = literal;
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

static Expr* PrattParseExpr(Lexer* lexer, Expr* expr, int minbp) {
	Expr* lhs = NULL;
	if (!expr) {
		Token* tlhs = Next(lexer);
		switch (tlhs->type) {
			case TT_NUMBER: { 
				lhs = makeExpr(ET_INT_LITERAL);
				if (!ParseIntLiteral(lexer, tlhs, lhs))
					return NULL;

				break;
			}
			case TT_IDENT: {
				lhs = makeExpr(ET_IDENT);
				if (!ParseIdent(lexer, tlhs, lhs))
					return NULL;
				break;
			}
			default: {
				if (IsPrefixOperator(tlhs->type)) {
					int r_bp = prefix_binding_power[OpToPrefixIndex(tlhs->type)];
					Expr* operand = PrattParseExpr(lexer, expr, r_bp);
					if (!operand)
						return NULL;

					lhs = makeExpr(ET_UNARY_OP);
					lhs->unop = malloc(sizeof(UnaryOp));
					lhs->unop->type = OpToUnop(tlhs->type);
					lhs->unop->operand = operand;
					lhs->unop->loc.line = tlhs->line;
					lhs->unop->loc.pos = tlhs->pos;
					break;
				}

				ParserError(lexer, tlhs, "Expected literal/identifier/'+'/'-' here");
				return NULL;
			}
		}

		lhs->loc.line = tlhs->line;
		lhs->loc.pos = tlhs->pos;

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

		uint16_t bp = infix_binding_power[OpToInfixIndex(op->type)];
		uint16_t l_bp = bp >> 8, r_bp = bp & 0xff;
		
		if (l_bp < minbp)
			break;

		Next(lexer);
		Expr* rhs = PrattParseExpr(lexer, NULL, r_bp);
		if (!rhs)
			return NULL;

		Expr* tmp = makeExpr(ET_BINARY_OP);
		tmp->loc.line = op->line;
		tmp->loc.pos = op->pos;

		tmp->binop = malloc(sizeof(BinaryOp));
		tmp->binop->loc.line = op->line;
		tmp->binop->loc.pos = op->pos;
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
	if (!expr)
		goto no_init;

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

void DumpExpr(Expr* expr) {
	if (expr->type == ET_INT_LITERAL)
		printf("%d ", expr->literal);
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

