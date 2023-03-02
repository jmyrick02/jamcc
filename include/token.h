#define MAX_IDENTIFIER_LENGTH 512

#pragma once
typedef enum {
  PLUS = 0, 
  MINUS, 
  STAR, 
  SLASH, 
  BITSHIFT_LEFT,
  BITSHIFT_RIGHT,
  INTEGER_LITERAL,
  PRINT,
  FACTORIAL,
  SEMICOLON,
  INT,
  IDENTIFIER,
  LEFTVALUE_IDENTIFIER,
  ASSIGN,
  EQ,
  NEQ,
  LT,
  LEQ,
  GT,
  GEQ,
  END,
} TokenType;

#pragma once
static const int PRECEDENCE[] = {
  3, // PLUS
  3, // MINUS
  4, // STAR
  4, // SLASH
  2, // BITSHIFT_LEFT
  2, // BITSHIFT_RIGHT
  -1, // INTEGER_LITERAL
  -1, // PRINT
  -1, // FACTORIAL
  -1, // SEMICOLON
  -1, // INT
  -1, // IDENTIFIER
  -1, // LEFTVALUE_IDENTIFIER
  -1, // ASSIGN
  0, // EQ
  0, // NEQ
  1, // LT
  1, // LEQ
  1, // GT
  1, // GEQ
  -1, // END 
};

#pragma once
static const char* TOKENTYPE_STRING[] = {
  "+", // PLUS
  "-", // MINUS
  "*", // STAR
  "/", // SLASH
  "<<", // BITSHIFT_LEFT
  ">>", // BITSHIFT_RIGHT
  "integer literal", // INTEGER_LITERAL
  "print", // PRINT
  "factorial", // FACTORIAL
  "int", // INT
  "identifier", // IDENTIFIER
  "leftvalue identifier", // LEFTVALUE_IDENTIFIER
  "assign", // ASSIGN
  "==", // EQ
  "!=", // NEQ
  "<", // LT
  "<=", // LEQ
  ">", // GT
  ">=", // GEQ
  ";", // SEMICOLON
  "EOF", // END
};

#pragma once
typedef union TokenVal {
  int integer;
  char string[MAX_IDENTIFIER_LENGTH + 1];
} TokenVal;

#pragma once
typedef struct Token {
  TokenType type;
  TokenVal val;
} Token;
