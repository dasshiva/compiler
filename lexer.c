#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <assert.h>

#if defined(__linux__) || defined(__ANDROID__)
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#endif

#include "lexer.h"

static void die(const char* msg) {
	printf("%s\n");
	exit(1);
}

LexerError NewLexer(const char* name, Lexer* lexer) {
	if (!name || !lexer)
		return REQUIRED_PARAM_NULL;

	int len = strlen(name);
	if (len > PATH_MAX || !len)
		return INVALID_PATH;

#if defined(__linux__) || defined(__ANDROID__)
	struct stat st;
	if (stat(name, &st) < 0)
		return FILE_NOT_FOUND;

	if (st.st_size > (1 << 20))
		return FILE_TOO_LONG;

	if (!st.st_size)
		return EMPTY_FILE;

	int fd = open(name, O_RDONLY);
	if (fd < 0)
		return FILE_NOT_ACCESSIBLE;

	lexer->source = mmap(NULL, 	st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (lexer->source == MAP_FAILED)
		return FILE_MAP_FAILED;

	lexer->size = st.st_size;
#endif

	lexer->file = malloc((len + 1) * sizeof(char));
	if (!lexer->file)
		return OUT_OF_MEMORY;

	lexer->offset = 0;
	lexer->line = 1;
	lexer->pos = 1;
	lexer->peek = NULL;
	lexer->flags = 0;

	memcpy(lexer->file, name, len);
	lexer->file[len] ='\0';

	return SUCCESS;
}

LexerError DeleteLexer(Lexer* lexer) {
	if (!lexer)
		return REQUIRED_PARAM_NULL;

	free(lexer->file);
#if defined(__linux__) || defined(__ANDROID__)
	lexer->size -= lexer->size % 4096;
	lexer->size += 4096;

	if (munmap(lexer->source, lexer->size) < 0)
		return RESOURCE_CLEANUP_FAILED;
#endif

	return SUCCESS;
}

static Token* makeToken(Lexer* lexer, TokenType type, uint32_t length) {
	Token* token = malloc(sizeof(Token));
	if (!token)
		die("makeToken(): malloc() == NULL");

	token->type = type;
	token->line = lexer->line;
	token->pos = lexer->pos;
	token->offset = lexer->offset;
	token->length = length;

	if (length > 0) {
		lexer->pos += length;
		lexer->offset += length;
	}

	return token;
}

static int IsEOF(Lexer* lexer) {
	return lexer->offset >= lexer->size;
}

static void SkipSpace(Lexer* lexer) {
	while (!IsEOF(lexer)) {
		char c = lexer->source[lexer->offset];
		if (isspace(c)) {
			switch (c) {
				case '\r' : case '\f' :
				case ' '  : lexer->pos++; break;
				case '\n' : lexer->line++; lexer->pos = 1; break;
				case '\t' : {
					// Assume that tab stop is 8 spaces
					lexer->pos -= lexer->pos % 8;
					lexer->pos += 8;
					break;
				}
				case '\v' : lexer->line++; break;
				default: {} // unreachable
			}

		} else 
			return;

		lexer->offset++;
	}
}

/* The length of keywords, klen and kmap, must be equal */
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// List of all keywords
static const char* keywords[] = {
	"let", NULL
};

int lkeywords = ARRAY_SIZE(keywords);

/* Lengths of all keywords, indexed by their position
 * in keywords[] i.e klen[i] = strlen(keywords[i]) */
static int klen[] = {
	3, 0
};

/* What token type each keyword in keywords[] maps to
 * i.e kmap[i] = TokenType(keywords[i]) */
static TokenType kmap[] = {
	TT_LET, 0 
};

static Token* ReadIdentOrKeyword(Lexer* lexer) {
	// Make sure these hold or we will fail at run-time catastrophically
	_Static_assert(ARRAY_SIZE(keywords) == ARRAY_SIZE(klen), 
			"len(keywords) != len(klen)");
	_Static_assert(ARRAY_SIZE(keywords) == ARRAY_SIZE(kmap),
			"len(keywords) != len(kmap)");

	// Assume that it's an identifier, if it turns out to be a keyword
	// we can fix up token->type later on.
	
	Token* token = makeToken(lexer, TT_IDENT, 0);

	while (!IsEOF(lexer)) {
		char c = lexer->source[lexer->offset];
		if (isalnum(c)) {
			token->length++;
			lexer->pos++;
			lexer->offset++;
		}
		else 
			break;
	}

	for (int i = 0; i < lkeywords; i++) {
		if (klen[i] == token->length) {
			if (strncmp(keywords[i], lexer->source + token->offset, 
					klen[i]) == 0) {
				token->type = kmap[i];
				break;
			}
		}
	}

	return token;
}

static Token* ReadNumber(Lexer* lexer) {
	Token* token = makeToken(lexer, TT_NUMBER, 0);

	/* The parser will later figure out if this is at all a valid number
	 * or not, the lexer will consume everything alphanumeric, this allows
	 * us to offload the burden on figuring out the numeral system 
	 * (hexadecimal, octal, decimal etc.) onto the parser */
	
	while (!IsEOF(lexer)) {
		char c = lexer->source[lexer->offset];
		if (isalnum(c)) {
			token->length++;
			lexer->pos++;
			lexer->offset++;
		}
		else 
			break;
	}

	return token;
}

Token* Next(Lexer* lexer) {
	if (lexer->peek) {
		Token* ret = lexer->peek;
		lexer->peek = NULL;
		return ret;
	}

	if (IsEOF(lexer)) 
		return makeToken(lexer, TT_EOF, 0);

	SkipSpace(lexer);

	if (IsEOF(lexer))
		return makeToken(lexer, TT_EOF, 0);

	char c = lexer->source[lexer->offset];
	switch (c) {
		case '+' : return makeToken(lexer, TT_PLUS, 1);
		case '-' : return makeToken(lexer, TT_MINUS, 1);
		case '*' : return makeToken(lexer, TT_ASTERISK, 1);
		case '/' : return makeToken(lexer, TT_SLASH, 1);
		case ';' : return makeToken(lexer, TT_SEMICOLON, 1);
		default: {
			if (isdigit(c))
				return ReadNumber(lexer);

			if (isalpha(c))
				return ReadIdentOrKeyword(lexer);
		}
	}

	printf("Unknown token at %s %u:%u", lexer->file, lexer->line, lexer->pos);
	die("");
}

Token* Peek(Lexer* lexer) {
	if (lexer->peek)
		return lexer->peek;

	lexer->peek = Next(lexer);
	return lexer->peek;
}

static const char* T2S[] = {
	"EOF", "Number", "+", "-", "*", "/", ";", 
	"let", "Identifier"
};

void DumpToken(Lexer* lexer, Token* token) {
	// printf("{ line = %u, pos = %u, offset = %u, length = %u, ",
	// 		token->line, token->pos, token->offset, token->length);
	const char* desc = T2S[token->type];
	if (token->type != TT_NUMBER && token->type != TT_IDENT)
		printf("(%s)", desc);
	else {
		char* n = lexer->source + token->offset;
		char* text = malloc((token->length + 1) * sizeof(char));
		memcpy(text, n, token->length);
		text[token->length] = '\0';
		printf("%s ", text);
		free(text);
	}
}

