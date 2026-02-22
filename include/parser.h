#ifndef __PARSER_H__
#define __PARSER_H__

#include "lexer.h"
#include "operators.h"
#include "types.h"

typedef struct Location {
	uint32_t line;
	uint32_t pos;
} Location;

struct Expr;
typedef struct BinaryOp {
	OperatorCode type;
	struct Expr* left;
	struct Expr* right;
	Location loc; 
} BinaryOp;

typedef struct UnaryOp {
	OperatorCode type;
	struct Expr* operand;
	Location loc;
} UnaryOp;

enum ExprType {
	ET_INT_LITERAL,
	ET_IDENT,
	ET_UNARY_OP,
	ET_BINARY_OP,
	ET_CAST
};

typedef struct IntLiteral {
	Type* type;
	int64_t  number;
} IntLiteral;

typedef struct Cast {
	Type* target;
	struct Expr* expr;
} Cast;

typedef struct Expr {
	enum ExprType type;
	union {
		IntLiteral* literal;
		char* ident;
		UnaryOp* unop;
		BinaryOp* binop;
		Cast* cast;
	};
	Location loc;
} Expr;

enum StatementType {
	ST_EXPR,
	ST_VARDECL,
	ST_FUNCTION
};

typedef struct VarDecl {
	const char* ident;
	const char* type;
	Expr*  init;
	Location loc;
	Location loc_type;
} VarDecl;

typedef struct FuncArgs {
	const char* type;
	const char* name;
} FuncArgs;

typedef struct Function {
	const char* name;
	const char* rtype;
	Vector* args;
	Vector* statements;
} Function;

typedef struct Statement {
	enum StatementType type;
	union {
		Expr* expr;
		VarDecl* vardecl;
		Function* func;
	};

	Location loc;
} Statement;

Statement* ParseStatement(Lexer* lexer);
void DumpExpr(Expr* expr); 
void DumpStatement(Statement* stat); 
#endif
