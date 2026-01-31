#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdint.h>

typedef enum LexerError {
	SUCCESS,
	REQUIRED_PARAM_NULL = 1,
	INVALID_PATH,
	OUT_OF_MEMORY,
	FILE_NOT_FOUND,
	FILE_NOT_ACCESSIBLE,
	FILE_TOO_LONG,
	FILE_MAP_FAILED,
	EMPTY_FILE,
	RESOURCE_CLEANUP_FAILED
} LexerError;

/*  Represents the current state of the lexer 
 *  This lexer is designed to work with source files < 4GB, by design
 *  because generally, source files should not even approach this size by a
 *  wide margin. Furthermore, NewLexer will reject files that are greater 
 *  than 1MB in size, because most source files will rarely ever exceed
 *  this size. Moreover since the lexer relies on mapping files to memory, 
 *  we cannot have too many huge files in memory at any time */

typedef enum TokenType {
	TT_EOF,
	TT_NUMBER,
	TT_PLUS, TT_MINUS, TT_ASTERISK, TT_SLASH, TT_SEMICOLON,
	TT_LET, TT_IDENT, TT_EQUALS
} TokenType;

typedef struct Token {
	enum TokenType type;
	uint32_t line;
	uint32_t pos;
	uint32_t offset;
	uint32_t length;
} Token; 

typedef struct Lexer {
	char*    source; // the code from the file - mapped into memory
	char*    file;   // the name of the file that the lexer is working on
	Token*   peek;   // the next token filled in by a call to Peek()
	uint32_t offset; // offset into Lexer.source we are currently at
	uint32_t size;   // the size of the file we are working on, in bytes
	uint32_t line;   // the line we are at in the code (lines end with '\n')
	uint32_t pos;    // the position we are at, within the line
	uint32_t flags;  // Additional flags of the lexer state
} Lexer;

LexerError NewLexer(const char* name, Lexer* lexer);
Token* Next(Lexer* lexer);
Token* Peek(Lexer* lexer);
void DumpToken(Lexer* lexer, Token* token); 
LexerError DeleteLexer(Lexer* lexer);

#endif
