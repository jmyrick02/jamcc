#include <stdlib.h>
#include <string.h>

#include "../include/scan.h"
#include "../include/lib/logging.h"

Token GLOBAL_TOKEN;

const int MAX_INTEGER_LITERAL_DIGITS = 19;

// Get next valid character from file
char next(FILE* fp) {
	char c = fgetc(fp);

	return c;
}

// Gets the next non-whitespace character from file
char nextNonWhitespace(FILE* fp) {
	char c;

	do {
		c = next(fp);
	} while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

	return c;
}

// Scan integer literals into int objects
int scanIntegerLiteral(FILE* fp, char c) {
	char integer_buffer[MAX_INTEGER_LITERAL_DIGITS + 1];
	int buffer_index = 0;

	// while buffer is free and c is an integer
	while (buffer_index < MAX_INTEGER_LITERAL_DIGITS && c - '0' >= 0 && c - '9' <= 0) {
		integer_buffer[buffer_index] = c;
		buffer_index++;
		c = next(fp);
	}

	// Put back non-integer character
	ungetc(c, fp);

	return atoi(integer_buffer);
}

// Scan into GLOBAL_TOKEN
void scan(FILE* fp) {
	Token token;

	char c = nextNonWhitespace(fp);

	switch (c) {
		case EOF:
			token.type = END;
			break;
		case '+':
			token.type = PLUS;
			break;
		case '-':
			token.type = MINUS;
			break;
		case '*':
			token.type = STAR;
			break;
		case '/':
			token.type = SLASH;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			token.type = INTEGER_LITERAL;
			token.val = scanIntegerLiteral(fp, c);
			break;
		default:
			fatal(RC_ERROR, "Invalid token '%c'\n", c);
			break;
	}
	
	GLOBAL_TOKEN = token;
}

