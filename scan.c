#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"

const int MAX_INTEGER_LITERAL_DIGITS = 19; // taken from Purple - why? TODO

// Get next valid character from file
char next(FILE* fp) {
	char c = fgetc(fp);

	return c;
}

// Gets the next non-whitespace character from file
char next_nonwhitespace(FILE* fp) {
	char c;

	do {
		c = next(fp);
	} while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

	return c;
}

// Scan integer literals into int objects
int scan_integer_literal(FILE* fp, char c) {
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

// Scan into the next token
int scan(FILE* fp, Token* currentToken) {
	char c = next_nonwhitespace(fp);

	// Check for end of file
	/*
	if (c == '') {
		return 0;
	}
	*/

	switch (c) {
		case EOF:
			return 0;
		case '+':
			currentToken->type = PLUS;
			break;
		case '-':
			currentToken->type = MINUS;
			break;
		case '*':
			currentToken->type = STAR;
			break;
		case '/':
			currentToken->type = SLASH;
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
			currentToken->type = INTEGER_LITERAL;
			currentToken->value = scan_integer_literal(fp, c);
			break;
		default:
			currentToken->type = UNKNOWN_TOKEN;
			break;
	}

	return 1;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("ERROR - you didn't pass in an input file path!\n");
		return 0;
	}
	char* filepath = argv[1];

	FILE* fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("ERROR - failed to open file at ");
		printf(filepath);
		printf("\n");
	}

	// Scan file and print out its Tokens
	Token token; 

	while (scan(fp, &token)) {
		printf("Token type: %s", TOKENTYPE_STRING[token.type]);
		if (token.type == INTEGER_LITERAL) {
			printf("\nToken value: ");
			printf("%d", token.value);
		}
		printf("\n\n");
	}
}
