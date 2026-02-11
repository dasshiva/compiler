#ifndef __PARSER_UTILS_H__
#define __PARSER_UTILS_H__

#include "parser.h"

int IsOperator(TokenType type); 
int IsPrefixOperator(TokenType type);
enum UnaryOpType OpToUnop(TokenType type);
enum BinaryOpType OpToBinop(TokenType type);
Expr* makeExpr(enum ExprType type);
void ParserError(Lexer* lexer, Token* token, const char* msg);
int Expect(Lexer* lexer, TokenType type, const char* msg);
int ToDecimal(int digit, int base); 
#endif
