#include <stdlib.h>
#include <string.h>

#include "../include/scan.h"
#include "../include/lib/logging.h"

#define MAX_INTEGER_LITERAL_DIGITS 19

#define MAX_SHORT_VALUE 32767
#define MAX_INT_VALUE 2147483647
#define MAX_LONG_VALUE 9223372036854775807

FILE* GLOBAL_FILE_POINTER;
Token GLOBAL_TOKEN;

int CUR_LINENUM = 1;

// Get next valid character from file
char next() {
  return fgetc(GLOBAL_FILE_POINTER);
}

// Gets the next non-whitespace character from file
char nextNonWhitespace() {
  char c;
  do {
    c = next();
    if (c == '\n')
      CUR_LINENUM++;
  } while (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f');

  return c;
}

// Returns true if it is a comment
int scanComment() {
  char c = next();
  if (c != '/') { // This isn't a comment
    ungetc(c, GLOBAL_FILE_POINTER);
    return 0;
  }

  // This is a comment - ignore characters until end of line
  while (c != '\n') {
    c = next();
  }
  CUR_LINENUM++;
  return 1;
}

// Pass in the current scanned character
// Returns true if this is a bitshift operator
int scanBitshiftOperator(char c) {
  if (next() != c) {
    ungetc(c, GLOBAL_FILE_POINTER);
    return 0;
  }
   
  return 1;
}

// Scan integer literals into int objects
long scanIntegerLiteral(char c, NumberType* numTypeOut) {
  char integerBuffer[MAX_INTEGER_LITERAL_DIGITS + 1];
  int bufferIndex = 0;

  // while buffer is free and c is an integer
  while (bufferIndex < MAX_INTEGER_LITERAL_DIGITS && c - '0' >= 0 && c - '9' <= 0) {
    integerBuffer[bufferIndex] = c;
    bufferIndex++;
    c = next();
  }
  integerBuffer[bufferIndex] = '\0';
  
  long val = strtol(integerBuffer, NULL, 0);

  if (c == 'S') { // Short literal
    *numTypeOut = NUM_SHORT;
    if (val > MAX_SHORT_VALUE)
      fatal(RC_ERROR, "Literal value %s is too large for a short\n", integerBuffer);
    
    c = next();
  } else if (c == 'L') { // Long literal
    *numTypeOut = NUM_LONG;
    c = next();
  } else { // Integer literal
    *numTypeOut = NUM_INT;
    if (val > MAX_INT_VALUE)
      fatal(RC_ERROR, "Literal value %s is too large for an int\n", integerBuffer);
  }

  // Put back non-integer character
  ungetc(c, GLOBAL_FILE_POINTER);

  return val;
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

// Returns true if '==' - EQ operator
int scanEq() {
  char c = next();
  if (c != '=') { // This isn't ==
    ungetc(c, GLOBAL_FILE_POINTER);
    return 0;
  }

  return 1;
}

// Returns true if '<='
int scanLeqOrGeq() {
  char c = next();
  if (c != '=') { // This isn't <= or >=
    ungetc(c, GLOBAL_FILE_POINTER);
    return 0;
  }

  return 1;
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
      if (scanComment()) {
        scan();
        return;
      } else {
        token.type = SLASH;
      }
      break;
    case '<':
      if (scanBitshiftOperator(c)) {
        token.type = BITSHIFT_LEFT;
      } else if (scanLeqOrGeq()) {
        token.type = LEQ;
      } else {
        token.type = LT;
      }
      break;
    case '>':
      if (scanBitshiftOperator(c)) {
        token.type = BITSHIFT_RIGHT;
      } else if (scanLeqOrGeq()) {
        token.type = GEQ;
      } else {
        token.type = GT;
      }
      break;
    case '=':
      if (scanEq()) {
        token.type = EQ;
      } else {
        token.type = ASSIGN;
      }
      break;
    case '!':
      token.type = NEQ;
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
      token.type = NUMBER_LITERAL; // TODO
      token.val = (TokenVal) { .num = scanIntegerLiteral(c, &token.numType) };
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
        const char KEYWORD_PRINT[MAX_IDENTIFIER_LENGTH + 1] = "print";
        const char KEYWORD_FACTORIAL[MAX_IDENTIFIER_LENGTH + 1] = "factorial"; 
        const char KEYWORD_INT[MAX_IDENTIFIER_LENGTH + 1] = "int";
        if (strcmp(identifierBuffer, "print") == 0) {
          token.type = PRINT;
        } else if (strcmp(identifierBuffer, "factorial") == 0) {
          token.type = FACTORIAL;
        } else if (strcmp(identifierBuffer, "short") == 0) {
          token.type = SHORT;
        } else if (strcmp(identifierBuffer, "int") == 0) {
          token.type = INT;
        } else if (strcmp(identifierBuffer, "long") == 0) {
          token.type = LONG;
        } else {
          token.type = IDENTIFIER;
          switch (GLOBAL_TOKEN.type) {
            case SHORT:
              token.numType = NUM_SHORT;
              updateSymbolTable(identifierBuffer, -1, NUM_SHORT);
              break;
            case INT:
              token.numType = NUM_INT;
              updateSymbolTable(identifierBuffer, -1, NUM_INT);
              break;
            case LONG:
              token.numType = NUM_LONG;
              updateSymbolTable(identifierBuffer, -1, NUM_LONG);
              break;
            default: // Check symbol table
              {
                SymbolTableEntry* entry = getSymbolTableEntry(identifierBuffer);
                if (entry == NULL)
                  fatal(RC_ERROR, "Invalid variable declaration");
                token.numType = entry->numType;
              }
              break;
          }
          TokenVal val;
          strcpy(val.string, identifierBuffer);
          token.val = val;
        }
      }
      break;
    default:
      fatal(RC_ERROR, "Invalid token '%c'", c);
      break;
  }
  
  GLOBAL_TOKEN = token;
}

