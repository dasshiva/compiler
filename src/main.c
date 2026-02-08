#include "irgen.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
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

		DumpStatement(stat);
		printf("\n");
	}

	if (!VectorLength(stats))
		return 0;

	if (getenv("LANG_PARSE_ONLY"))
		return 0;

	Vector* symtab = SemanticAnalyse(&lexer, stats);
	if (!symtab)
		return 2;

	if (getenv("LANG_SEMA_ONLY"))
		return 0;

	Vector* ir = GenIR(stats, symtab);
	if (!ir) {
		printf("Internal Error: IR Generation failed\n");
		return 3;
	}

	printf("IR Instructions = %u\n", VectorLength(ir));
	PrintIR(ir);

	DeleteLexer(&lexer);
	return 0;
}
