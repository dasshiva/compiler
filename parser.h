#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

enum BinaryOpType {
	BOT_ADD, BOT_SUB,
	BOT_MUL, BOT_DIV
};

struct Expr;
typedef struct BinaryOp {
	enum BinaryOpType type;
	struct Expr* left;
	struct Expr* right;
} BinaryOp;

enum ExprType {
	ET_LITERAL,
	ET_BINARY_OP
};

typedef struct Expr {
	enum ExprType type;
	union {
		Token* literal;
		BinaryOp* binop;
	};
} Expr;

Expr* ParseStatement(Lexer* lexer);
void DumpExpr(Lexer* lexer, Expr* expr); 

#endif
