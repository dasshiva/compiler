#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

typedef struct Location {
	uint32_t line;
	uint32_t pos;
} Location;

enum BinaryOpType {
	BOT_ADD, BOT_SUB,
	BOT_MUL, BOT_DIV, BOT_EQUALS, BOT_MOD
};

struct Expr;
typedef struct BinaryOp {
	enum BinaryOpType type;
	struct Expr* left;
	struct Expr* right;
	Location loc; 
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
	Location loc;
} Expr;

enum StatementType {
	ST_EXPR,
	ST_VARDECL
};

typedef struct VarDecl {
	Token* ident;
	Token* type;
	Expr*  init;
	Location loc;
} VarDecl;

typedef struct Statement {
	enum StatementType type;
	union {
		Expr* expr;
		VarDecl* vardecl;
	};

	Location loc;
} Statement;

Statement* ParseStatement(Lexer* lexer);
void DumpExpr(Lexer* lexer, Expr* expr); 
void DumpStatement(Lexer* lexer, Statement* stat); 
#endif
