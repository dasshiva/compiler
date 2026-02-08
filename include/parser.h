#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"

typedef struct Location {
	uint32_t line;
	uint32_t pos;
} Location;

enum BinaryOpType {
	BOT_ADD, BOT_SUB,
	BOT_MUL, BOT_DIV, BOT_EQUALS, BOT_MOD,
	BOT_MAX
};

enum UnaryOpType {
	UOT_ADD, UOT_MINUS, UOT_MAX
};

struct Expr;
typedef struct BinaryOp {
	enum BinaryOpType type;
	struct Expr* left;
	struct Expr* right;
	Location loc; 
} BinaryOp;

typedef struct UnaryOp {
	enum UnaryOpType type;
	struct Expr* operand;
	Location loc;
} UnaryOp;

enum ExprType {
	ET_INT_LITERAL,
	ET_IDENT,
	ET_UNARY_OP,
	ET_BINARY_OP
};

typedef struct Expr {
	enum ExprType type;
	union {
		int literal;
		char* ident;
		UnaryOp* unop;
		BinaryOp* binop;
	};
	Location loc;
} Expr;

enum StatementType {
	ST_EXPR,
	ST_VARDECL
};

typedef struct VarDecl {
	const char* ident;
	const char* type;
	Expr*  init;
	Location loc;
	Location loc_type;
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
void DumpExpr(Expr* expr); 
void DumpStatement(Statement* stat); 
#endif
