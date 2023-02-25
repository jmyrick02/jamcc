#include <stdlib.h>
#include <string.h>

#include "../include/scan.h"
#include "../include/lib/logging.h"

#define MAX_INTEGER_LITERAL_DIGITS 19
#define MAX_IDENTIFIER_LENGTH 512

FILE* GLOBAL_FILE_POINTER;
Token GLOBAL_TOKEN;

// Get next valid character from file
char next() {
  return fgetc(GLOBAL_FILE_POINTER);
}

// Gets the next non-whitespace character from file
char nextNonWhitespace() {
  char c;
  do {
    c = next();
  } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

  return c;
}

void scanBitshiftOperator() {
  next();
}

// Scan integer literals into int objects
int scanIntegerLiteral(char c) {
  char integerBuffer[MAX_INTEGER_LITERAL_DIGITS + 1];
  int bufferIndex = 0;

  // while buffer is free and c is an integer
  while (bufferIndex < MAX_INTEGER_LITERAL_DIGITS && c - '0' >= 0 && c - '9' <= 0) {
    integerBuffer[bufferIndex] = c;
    bufferIndex++;
    c = next();
  }
  integerBuffer[bufferIndex] = '\0';

  // Put back non-integer character
  ungetc(c, GLOBAL_FILE_POINTER);

  return atoi(integerBuffer);
}

// Scan identifier into buffer with space maxLength
void scanIdentifier(char c, char* buffer, int maxLength) {
  int buffer_index = 0;

  // while buffer is free and c is alphanumeric or '_'
  while (buffer_index < maxLength && (c == '_' || (c - 'A' >= 0 && c - 'z' <= 0) || (c - '0' >= 0 && c - '9' <= 0))) {
    buffer[buffer_index] = c;
    buffer_index++;
    c = next();
  }
  buffer[buffer_index] = '\0';

  // Put back non-alphanumeric/_ character
  ungetc(c, GLOBAL_FILE_POINTER);
}

// Scan into GLOBAL_TOKEN
void scan() {
  Token token;

  char c = nextNonWhitespace();
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
    case '<':
      token.type = BITSHIFT_LEFT;
      scanBitshiftOperator();
      break;
    case '>':
      token.type = BITSHIFT_RIGHT;
      scanBitshiftOperator();
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
      token.val = scanIntegerLiteral(c); 
      break;
    case ';':
      token.type = SEMICOLON;
      break;
    case '_':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
      {
        char identifierBuffer[MAX_IDENTIFIER_LENGTH + 1];
        scanIdentifier(c, identifierBuffer, MAX_IDENTIFIER_LENGTH);
        
        // Check keywords
        const char KEYWORD_PRINT[MAX_IDENTIFIER_LENGTH] = "print";
        const char KEYWORD_FACTORIAL[MAX_IDENTIFIER_LENGTH] = "factorial"; 
        if (strcmp(identifierBuffer, KEYWORD_PRINT) == 0) {
          token.type = PRINT;
        } else if (strcmp(identifierBuffer, KEYWORD_FACTORIAL) == 0) {
          token.type = FACTORIAL;
        } else {
          fatal(RC_ERROR, "Unrecognized identifier \"%s\"", identifierBuffer);
        }
      }
      break;
    default:
      fatal(RC_ERROR, "Invalid token '%c'", c);
      break;
  }
  
  GLOBAL_TOKEN = token;
}

