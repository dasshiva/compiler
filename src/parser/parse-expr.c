#include <stdlib.h>
#include <string.h>
#include "parser-utils.h"
#include "parse-expr.h"
#include <stdio.h>

int ParseIntLiteral(Lexer* lexer, Token* token, Expr* expr);
int ParseIdent(Lexer* lexer, Token* token, Expr* expr);

static uint16_t infix_binding_power[] = {
	(2 << 8) | 1, // =
	(11 << 8) | 12, // + , -
	(13 << 8) | 14, // * , /, %
	(31 << 8) | 32, // function call ()
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
		case TT_LPAREN: return 3; // function call
		default: return -1;
	}
}

static Vector* ParseFunCall(Lexer* lexer, Token* callop) {
	Vector* args = NewVector();

	while (1) {
		Token* token = Peek(lexer);
		if (token->type == TT_RPAREN) {
			Next(lexer);
			break;
		}

		Expr* expr = PrattParseExpr(lexer, NULL, 0);
		if (!expr) {
			ParserError(lexer, token, "Expected valid expression here");
			return NULL;
		}

		Append(args, expr);
		token = Peek(lexer);
		if (token->type == TT_RPAREN) {
			Next(lexer);
			break;
		}

		else if (token->type == TT_COMMA) {
			Next(lexer);
			token = Peek(lexer);
			if (token->type == TT_RPAREN) {
				ParserError(lexer, token, 
					"Parameters expected after ','");
				return NULL;
			}
		}
		else {
			ParserError(lexer, token, "Expected ','/'')' here");
			return NULL;
		}
	}

	// For the moment, function calls are only the built-in type casts, 
	// so there should only be 1 argument
	if (VectorLength(args) != 1) {
		ParserError(lexer, callop, "Only 1 argument is needed");
		return NULL;
	}

	return args;
}

Type* ValidateFunCall(Lexer* lexer, Expr* name, Token* tname) {
	if (name->type != ET_IDENT) {
		ParserError(lexer, tname, "Expected identifier before '('");
		return NULL;
	}

	Type* totype = NULL;

	for (int i = 0; i < len_builtins; i++) {
		if (strcmp(BUILTIN_TYPES[i]->name, name->ident) == 0) {
			totype = BUILTIN_TYPES[i];
			break;
		}
	}

	if (!totype) {
		ParserError(lexer, tname, "Unknown type name in cast expression");
		return NULL;
	}

	return totype;
}

static Expr* ParsePredicate(Lexer* lexer) { 
	Token* tlhs = Next(lexer);
	Expr* lhs = NULL;

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

		case TT_LPAREN: {
			Expr* paren_expr = PrattParseExpr(lexer, NULL, 0);
			if (!paren_expr) 
				return NULL;

			if (!Expect(lexer, TT_RPAREN, "Expected ')' to end "
							"parenthesized expression"))
					return NULL;

			lhs = paren_expr;
			break;
		}

			
		default: {
			if (IsPrefixOperator(tlhs->type)) {
					
				int r_bp = prefix_binding_power[OpToPrefixIndex(tlhs->type)];
				Expr* operand = PrattParseExpr(lexer, NULL, r_bp);
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

			ParserError(lexer, tlhs, "Expected literal/identifier/" 
						"'+'/'-' or '(' here");
			return NULL;
		}
	}

	if (lhs) {
		lhs->loc.line = tlhs->line;
		lhs->loc.pos = tlhs->pos;
	}

	return lhs;
}

Expr* PrattParseExpr(Lexer* lexer, Expr* expr, int minbp) {
	Expr* lhs = NULL;
	if (!expr) {
		lhs = ParsePredicate(lexer);
		if (!lhs) 
			return NULL;

	}  else { 
		lhs = expr;
	}
	
	while (1) {
		Token* op = Peek(lexer);
		if (op->type == TT_SEMICOLON || op->type == TT_EOF)
			break;

		if (!IsOperator(op->type))
			break;

		uint16_t bp = infix_binding_power[OpToInfixIndex(op->type)];
		uint16_t l_bp = bp >> 8, r_bp = bp & 0xff;
		
		if (l_bp < minbp)
			break;

		Next(lexer);
		Expr* rhs = NULL;

		if (op->type == TT_LPAREN) {
			Type* type = ValidateFunCall(lexer, lhs, op);
			if (!type)
				return NULL;

			Vector* args = ParseFunCall(lexer, op);
			if (!args)
				return NULL;

			rhs = makeExpr(ET_CAST);
			rhs->cast = malloc(sizeof(Cast));
			rhs->cast->target = type;
			rhs->cast->expr = Get(args, 0);
			lhs = rhs;
			lhs->loc.line = op->line;
			lhs->loc.pos = op->pos;
			continue;
		}
		else {
			rhs = PrattParseExpr(lexer, NULL, r_bp);
			if (!rhs)
				return NULL;
		}

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

