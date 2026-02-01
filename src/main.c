#include "sema.h"
#include <stddef.h>
#include <stdio.h>

int main(int argc, const char** argv) {
	if (argc < 2) 
		return 1;

	Lexer lexer; 
	if (NewLexer(argv[1], &lexer))
		return 1;

	Token* token = NULL;
	Vector* stats = NewVector();

	while (1) {
		token = Peek(&lexer);
		if (token->type == TT_EOF) {
			Next(&lexer);
			break;
		}

		Statement* stat = ParseStatement(&lexer);
		if (!stat)
			break;

		Append(stats, stat);

		DumpStatement(&lexer, stat);
		printf("\n");
	}

	if (!SemanticAnalyse(&lexer, stats))
		return 2;

	DeleteLexer(&lexer);
	return 0;
}
