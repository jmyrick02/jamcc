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
  FUNCTION,
  FUNCTION_CALL,
  RETURN,
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
  AMPERSAND,
  DEREFERENCE,
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
  4, // PLUS
  4, // MINUS
  5, // STAR
  5, // SLASH
  3, // BITSHIFT_LEFT
  3, // BITSHIFT_RIGHT
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
  -1, // FUNCTION
  -1, // FUNCTION_CALL
  -1, // RETURN
  0, // ASSIGN
  1, // EQ
  1, // NEQ
  2, // LT
  2, // LEQ
  2, // GT
  2, // GEQ
  -1, // IF
  -1, // ELSE
  -1, // WHILE
  -1, // FOR
  -1, // BREAK
  -1, // CONTINUE
  -1, // LABEL_TOKEN
  -1, // AMPERSAND
  -1, // DEREFERENCE
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
  "function", // FUNCTION
  "function call", // FUNCTION_CALL
  "return", // RETURN
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
  "ampersand", // AMPERSAND
  "*", // DEREFERENCE
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
typedef enum {
  FUNCTION_TYPE = 0,
  NUMBER_TYPE,
} TypeType;

#pragma once
typedef struct Function {
  TokenType returnType;
} Function;

#pragma once
typedef struct Number {
  NumberType numType;
  int registerNum;
  int pointerDepth;
} Number;

#pragma once
typedef union TypeValue {
  Number number;
  Function function;
} TypeValue;

#pragma once
typedef struct Type {
  TypeType type;
  TypeValue value;
} Type;

#pragma once
typedef struct Token {
  TokenType type;
  TokenVal val;
  Type valueType;
} Token;

// Constructors:

#define CONSTRUCTOR_NUMBER(num_type) (Number) {num_type, -1, 0}

#define CONSTRUCTOR_NUMBER_POINTER(num_type, pointer_depth) (Number) {num_type, -1, pointer_depth}

#define CONSTRUCTOR_NUMBER_REGISTER(num_type, register_num, pointer_depth) (Number) {num_type, register_num, pointer_depth}

#define CONSTRUCTOR_INT_TYPE (Type) {NUMBER_TYPE, CONSTRUCTOR_NUMBER(NUM_INT)}

#define CONSTRUCTOR_NUMBER_TYPE(num_type) (Type) {NUMBER_TYPE, CONSTRUCTOR_NUMBER(num_type)}

#define CONSTRUCTOR_NUMBER_POINTER_TYPE(num_type, pointer_depth) (Type) {NUMBER_TYPE, CONSTRUCTOR_NUMBER_POINTER(num_type, pointer_depth)}

#define CONSTRUCTOR_NUMBER_TYPE_FULL(num_type, register_num, pointer_depth) (Type) {NUMBER_TYPE, CONSTRUCTOR_NUMBER_REGISTER(num_type, register_num, pointer_depth)}

#define CONSTRUCTOR_FUNCTION_TYPE(return_type) (Type) {FUNCTION_TYPE, (TypeValue) { .function = (Function) {return_type} }}

#define CONSTRUCTOR_TOKENVAL_NUM(v) (TokenVal) { .num = v }

#define CONSTRUCTOR_TOKEN_EMPTY(token_type) (Token) {token_type, (TokenVal) {0}, CONSTRUCTOR_NUMBER_TYPE(NUM_INT)}

#define CONSTRUCTOR_TOKEN_WITH_NUMBER(token_type, num_type) (Token) {token_type, (TokenVal) {0}, CONSTRUCTOR_NUMBER_TYPE(num_type)}

#define CONSTRUCTOR_TOKEN_WITH_NUMBER_POINTER(token_type, num_type, pointer_depth) (Token) {token_type, (TokenVal) {0}, CONSTRUCTOR_NUMBER_POINTER_TYPE(num_type, pointer_depth)}

#define CONSTRUCTOR_TOKEN_CONSTANT(val, num_type) (Token) {NUMBER_LITERAL, (TokenVal) {val}, CONSTRUCTOR_NUMBER_TYPE(num_type)}

#define CONSTRUCTOR_TOKEN_LABEL(label_num) (Token) {LABEL_TOKEN, (TokenVal) {label_num}, CONSTRUCTOR_NUMBER_TYPE(NUM_INT)}

#define CONSTRUCTOR_TOKEN_FUNCTION_CALL(return_type) (Token) {FUNCTION_CALL, (TokenVal) {}, CONSTRUCTOR_FUNCTION_TYPE(return_type)}

#define CONSTRUCTOR_TOKEN_AMPERSAND(num_type, register_num, pointer_depth) (Token) {AMPERSAND, (TokenVal) {}, CONSTRUCTOR_NUMBER_TYPE_FULL(num_type, register_num, pointer_depth)}

#define CONSTRUCTOR_TOKEN_NUMBER_IDENTIFIER(num_type, pointer_depth) (Token) {IDENTIFIER, (TokenVal) {}, CONSTRUCTOR_NUMBER_POINTER_TYPE(num_type, pointer_depth)}

#define CONSTRUCTOR_TOKEN_RETURN (Token) {RETURN, (TokenVal) {}, CONSTRUCTOR_NUMBER_TYPE(NUM_INT)}
