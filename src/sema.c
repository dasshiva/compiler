#include "sema.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

int SemaStatement(Statement* stat, Error* err) {
	MakeError(err, &stat->loc, "Test Error");
	return 0;
}

int SemanticAnalyse(Lexer* lexer, Vector* statements) {
	if (!VectorLength(statements))
		return 1;

	for (uint32_t i = 0; i < VectorLength(statements); i++) {
		Error err;
		if (!SemaStatement(Get(statements, i), &err))
			SemaError(lexer, &err);
	}

	return 0;
}
