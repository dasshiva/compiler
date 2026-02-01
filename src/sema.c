#include "sema.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"

typedef struct Error {
	Location* loc;
	char message[400];
} Error;


static void MakeError(Error* err, Location* loc, const char* msg, ...) {
	va_list ap;
	va_start(ap, msg);

	err->loc = loc;
	vsnprintf(err->message, 400, msg, ap);
	va_end(ap);
}

static void SemaError(Lexer* lexer, Error* err) {
	printf("%s:%u:%u\nError: %s\n", lexer->file, err->loc->line, 
			err->loc->pos, err->message);

	LineExtent* le = Get(lexer->extent, err->loc->line - 1);
	char* source = lexer->source + le->offset;
	char* line = malloc(sizeof(char) * (le->length + 1));
	memcpy(line, source, le->length);
	line[le->length] = '\0';

	printf("%s\n", line);
	for (uint32_t i = 0; i < err->loc->pos - 1; i++) { printf(" "); }
	printf("^\n");
	free(line);
}

enum SymbolType {
	TYPE_VARIABLE, // A variable declaration
	TYPE_TYPEDEF,  // A definition for a  type, user-defined or built-in
};

typedef struct Symbol {
	const char* name;
	enum SymbolType type;
	uint32_t flags;
	const Type* utype; // the underlying type, or an actual kind of type depending
				 // on the value of Symbol.type
} Symbol;

Symbol* MakeSymbol(const char* name, enum SymbolType type, uint32_t flags) {
	Symbol* ret = malloc(sizeof(Symbol));
	if (!ret)
		return NULL;

	ret->name = name;
	ret->type = type;
	ret->flags = flags;
}

int SemaExpression(Expr* expr, Vector* symtab, Error* err) {
	return 0;
}

const Type* GetType(Vector* symtab, const char* name) {
	for (uint32_t idx = 0; idx < VectorLength(symtab); idx++) {
		Symbol* sym = Get(symtab, idx);
		if (sym->type != TYPE_TYPEDEF)
			continue;

		if (strcmp(name, sym->name) == 0)
			return sym->utype;
	}

	return NULL;
}

int SemaVarDecl(Statement* stat, Vector* symtab, Error* err) {
	VarDecl* vardecl = stat->vardecl;
	const Type* ty;

	if (vardecl->type) {
		ty = GetType(symtab, vardecl->type);
		if (!ty) {
			MakeError(err, &stat->loc, "Unknown type name %s", vardecl->type);
			return 1;
		}
	}

	Symbol* sym = MakeSymbol(vardecl->ident, TYPE_VARIABLE, 0);
	sym->utype = ty;

	if (vardecl->init)
		return SemaExpression(vardecl->init, symtab, err);

	return 1;
}

int SemaStatement(Statement* stat, Vector* symtab, Error* err) {
	switch (stat->type) {
		case ST_VARDECL: return SemaVarDecl(stat, symtab, err);
		case ST_EXPR: return SemaExpression(stat->expr, symtab, err);
		default: {
			printf("SemaStatement(): Unknown statement type\n");
			break;
		}
	}

	return 0;
}


int SemanticAnalyse(Lexer* lexer, Vector* statements) {
	if (!VectorLength(statements))
		return 1;

	Vector* symtab = NewVector();
	for (int i = 0; i < len_builtins; i++) {
		Symbol* sym = MakeSymbol(BUILTIN_TYPES[i]->name, TYPE_TYPEDEF, 0);
		sym->utype = BUILTIN_TYPES[i];
		Append(symtab, sym);
	}

	for (uint32_t i = 0; i < VectorLength(statements); i++) {
		Error err;
		if (!SemaStatement(Get(statements, i), symtab, &err))
			SemaError(lexer, &err);
	}

	return 0;
}
