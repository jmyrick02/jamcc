#define MAX_IDENTIFIER_LENGTH 512

#pragma once
typedef enum {
  PLUS = 0, 
  MINUS, 
  STAR, 
  SLASH, 
  BITSHIFT_LEFT,
  BITSHIFT_RIGHT,
  NUMBER_LITERAL,
  PRINT,
  FACTORIAL,
  SEMICOLON,
  SHORT,
  INT,
  LONG,
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
  -1, // NUMBER_LITERAL
  -1, // PRINT
  -1, // FACTORIAL
  -1, // SEMICOLON
  -1, // SHORT
  -1, // INT
  -1, // LONG
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
  "number literal", // NUMBER_LITERAL
  "print", // PRINT
  "factorial", // FACTORIAL
  ";", // SEMICOLON
  "short", // SHORT
  "int", // INT
  "long", // LONG
  "identifier", // IDENTIFIER
  "leftvalue identifier", // LEFTVALUE_IDENTIFIER
  "assign", // ASSIGN
  "==", // EQ
  "!=", // NEQ
  "<", // LT
  "<=", // LEQ
  ">", // GT
  ">=", // GEQ
  "EOF", // END
};

#pragma once
typedef union TokenVal {
  long num;
  char string[MAX_IDENTIFIER_LENGTH + 1];
} TokenVal;

#pragma once
typedef enum {
  NUM_BOOL = 0,
  NUM_SHORT,
  NUM_INT,
  NUM_LONG,
} NumberType;

#pragma once
static const int NUMBERTYPE_SIZE[] = {
  1, // NUM_BOOL
  16, // NUM_SHORT
  32, // NUM_INT
  64, // NUM_LONG
};

#pragma once
static const char* NUMBERTYPE_STRING[] = {
  "bool", // NUM_BOOL
  "short", // NUM_SHORT
  "int", // NUM_SHORT
  "long", // NUM_LONG
};

#pragma once
static const char* NUMBERTYPE_LLVM[] = {
  "i1", // NUM_BOOL
  "i16", // NUM_SHORT
  "i32", // NUM_INT
  "i64", // NUM_LONG
};

#pragma once
typedef struct Token {
  TokenType type;
  TokenVal val;
  NumberType numType;
} Token;
