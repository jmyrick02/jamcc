#define MAX_IDENTIFIER_LENGTH 512

#pragma once
typedef enum {
  UNKNOWN_TOKEN = 0,
  LEFT_BRACE,
  RIGHT_BRACE,
  LEFT_PAREN,
  RIGHT_PAREN,
  PLUS, 
  MINUS, 
  STAR, 
  SLASH, 
  BITSHIFT_LEFT,
  BITSHIFT_RIGHT,
  NUMBER_LITERAL,
  PRINT,
  FACTORIAL,
  SEMICOLON,
  VOID,
  SHORT,
  INT,
  LONG,
  CHAR,
  IDENTIFIER,
  LEFTVALUE_IDENTIFIER,
  FUNCTION,
  ASSIGN,
  EQ,
  NEQ,
  LT,
  LEQ,
  GT,
  GEQ,
  IF,
  ELSE,
  WHILE,
  FOR,
  BREAK,
  CONTINUE,
  LABEL_TOKEN,
  AST_GLUE,
  END,
} TokenType;

#pragma once
static const int PRECEDENCE[] = {
  -1, // UNKNOWN_TOKEN
  -1, // LEFT_BRACE
  -1, // RIGHT_BRACE
  -1, // LEFT_PAREN
  -1, // RIGHT_PAREN
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
  -1, // VOID
  -1, // SHORT
  -1, // INT
  -1, // LONG
  -1, // CHAR
  -1, // IDENTIFIER
  -1, // LEFTVALUE_IDENTIFIER
  -1, // FUNCTION
  -1, // ASSIGN
  0, // EQ
  0, // NEQ
  1, // LT
  1, // LEQ
  1, // GT
  1, // GEQ
  -1, // IF
  -1, // ELSE
  -1, // WHILE
  -1, // FOR
  -1, // BREAK
  -1, // CONTINUE
  -1, // LABEL_TOKEN
  -1, // AST_GLUE
  -1, // END 
};

#pragma once
static const char* TOKENTYPE_STRING[] = {
  "unknown token", // UNKNOWN_TOKEN
  "{", // LEFT_BRACE
  "}", // RIGHT_BRACE
  "(", // LEFT_PAREN
  ")", // RIGHT_PAREN
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
  "void", // VOID
  "short", // SHORT
  "int", // INT
  "long", // LONG
  "char", // CHAR
  "identifier", // IDENTIFIER
  "leftvalue identifier", // LEFTVALUE_IDENTIFIER
  "function", // FUNCTION
  "assign", // ASSIGN
  "==", // EQ
  "!=", // NEQ
  "<", // LT
  "<=", // LEQ
  ">", // GT
  ">=", // GEQ
  "if", // IF
  "else", // ELSE
  "while", // WHILE
  "for", // FOR
  "break", // BREAK
  "continue", // CONTINUE
  "label", // LABEL_TOKEN
  "ast glue", // AST_GLUE
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
  NUM_CHAR,
  NUM_SHORT,
  NUM_INT,
  NUM_LONG,
} NumberType;

#pragma once
static const int NUMBERTYPE_SIZE[] = {
  1, // NUM_BOOL
  8, // NUM_CHAR
  16, // NUM_SHORT
  32, // NUM_INT
  64, // NUM_LONG
};

#pragma once
static const char* NUMBERTYPE_STRING[] = {
  "bool", // NUM_BOOL
  "char", // NUM_CHAR
  "short", // NUM_SHORT
  "int", // NUM_SHORT
  "long", // NUM_LONG
};

#pragma once
static const char* NUMBERTYPE_LLVM[] = {
  "i1", // NUM_BOOL
  "i8", // NUM_CHAR
  "i16", // NUM_SHORT
  "i32", // NUM_INT
  "i64", // NUM_LONG
};

#pragma once
typedef struct Function {
  TokenType returnType;
} Function;

#pragma once
typedef struct Number {
  NumberType numType;
  int registerNum;
} Number;

#pragma once
typedef union Type {
  Number number;
  Function function;
} Type;

#pragma once
typedef struct Token {
  TokenType type;
  TokenVal val;
  Type valueType;
} Token;
