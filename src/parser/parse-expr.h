#ifndef __PARSE_EXPR_H__
#define __PARSE_EXPR_H__

#include "parser.h"

Expr* PrattParseExpr(Lexer* lexer, Expr* expr, int minbp);

#endif
