#ifndef __SEMA_H__
#define __SEMA_H__

#include "parser.h"
#include "types.h"

enum SymbolType {
	TYPE_VARIABLE, // A variable declaration
	TYPE_TYPEDEF,  // A definition for a  type, user-defined or built-in
};

typedef struct Symbol {
	const char* name;
	enum SymbolType type;
	uint32_t flags;
	Type* utype;
	void* data;
} Symbol;

Vector* SemanticAnalyse(Lexer* lexer, Vector* statements);
Type* GetType(Vector* symtab, const char* name);
Symbol* GetVariable(Vector* symtab, const char* name);

#endif
