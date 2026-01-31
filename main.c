#include "parser.h"
#include <stddef.h>
#include <stdio.h>

int main(int argc, const char** argv) {
	if (argc < 2) 
		return 1;

	Lexer lexer; 
	if (NewLexer(argv[1], &lexer))
		return 1;

	Token* token = NULL;
	while (1) {
		token = Peek(&lexer);
		if (token->type == TT_EOF) {
			Next(&lexer);
			break;
		}

		Statement* stat = ParseStatement(&lexer);
		if (!stat)
			break;

		DumpStatement(&lexer, stat);
		printf("\n");
	}

	DeleteLexer(&lexer);
	return 0;
}
