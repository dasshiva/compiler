#ifndef __PARSER_UTILS_H__
#define __PARSER_UTILS_H__

#include "parser.h"
#include "operators.h"

int IsOperator(TokenType type); 
int IsPrefixOperator(TokenType type);
OperatorCode OpToUnop(TokenType type);
OperatorCode OpToBinop(TokenType type);
Expr* makeExpr(enum ExprType type);
void ParserError(Lexer* lexer, Token* token, const char* msg);
int Expect(Lexer* lexer, TokenType type, const char* msg);
int ToDecimal(int digit, int base); 
#endif
