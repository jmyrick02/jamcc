#include "../../include/parsing/statement.h"

extern Token GLOBAL_TOKEN;

Token matchToken(TokenType type) {
  Token cur = GLOBAL_TOKEN;
  if (cur.type != type) {
    fatal(RC_ERROR, "Expected token type %s but got %s\n", TOKENTYPE_STRING[type], TOKENTYPE_STRING[cur.type]);
  }
  scan();
  return cur;
}

ASTNode* parsePrintStatement() {
  matchToken(PRINT);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {PRINT, 0};
  result->left = parseBinaryExpression(NUM_INT);
  result->right = NULL;

  if (result->left->token.numType != NUM_INT)
    fatal(RC_ERROR, "Print statements only work on integers, received %s\n", NUMBERTYPE_STRING[result->left->token.numType]);

  matchToken(SEMICOLON);

  return result;
}

ASTNode* parseFactorialStatement() {
  matchToken(FACTORIAL);
  Token literal = matchToken(NUMBER_LITERAL);
  if (literal.numType != NUM_INT)
    fatal(RC_ERROR, "Must pass in integer literal to factorial statements\n");

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {PRINT, 0};

  ASTNode* cur = malloc(sizeof(ASTNode));
  result->left = cur;
  result->right = NULL;
  cur->token = (Token) {STAR, 0};

  int num = literal.val.num;
  while (num > 1) {
    cur->left = malloc(sizeof(ASTNode));
    cur->left->token = (Token) {NUMBER_LITERAL, num, NUM_INT};
    cur->left->left = NULL;
    cur->left->right = NULL;

    cur->right = malloc(sizeof(ASTNode));
    cur->right->token = (Token) {STAR, 0};
    cur->right->left = NULL;
    cur->right->right = NULL;

    cur = cur->right;
    num--;
  }
  cur->token = (Token) {NUMBER_LITERAL, (TokenVal) { .num = 1 }, NUM_INT};

  matchToken(SEMICOLON);

  return result;
}

void parseDeclarationStatement() {
  Token cur = GLOBAL_TOKEN;
  if (cur.type != SHORT && cur.type != INT && cur.type != LONG)
    fatal(RC_ERROR, "Expected valid type declaration but received %s\n", TOKENTYPE_STRING[cur.type]);
  scan();

  Token identifier = matchToken(IDENTIFIER);

  NumberType type;
  switch (cur.type) {
    case SHORT:
      type = NUM_SHORT;
      break;
    case INT:
      type = NUM_INT;
      break;
    case LONG:
      type = NUM_LONG;
      break;
    default:
      fatal(RC_ERROR, "Invalid variable type %s\n", TOKENTYPE_STRING[cur.type]);
      break;
  }

  updateSymbolTable(identifier.val.string, -1, type);
  generateDeclareGlobal(identifier.val.string, 0, type);

  matchToken(SEMICOLON);
}

ASTNode* parseAssignmentStatement() {
  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry* entry = getSymbolTableEntry(identifier.val.string);
  if (entry == NULL)
    fatal(RC_ERROR, "Undefined variable %s\n", identifier.val.string);

  matchToken(ASSIGN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->left = parseBinaryExpression(GLOBAL_TOKEN.numType);
  result->right = malloc(sizeof(ASTNode));
  result->right->token = (Token) {LEFTVALUE_IDENTIFIER, (TokenVal) {}, entry->numType};
  strcpy(result->right->token.val.string, identifier.val.string);

  matchToken(SEMICOLON);

  return result;
}

ASTNode* parseEnd() {
  ASTNode* endNode = malloc(sizeof(ASTNode));
  endNode->token = (Token) {END, 0};
  return endNode;
}

ASTNode* parseStatement() {
  switch (GLOBAL_TOKEN.type) {
    case PRINT:
      return parsePrintStatement();
    case FACTORIAL:
      return parseFactorialStatement();
    case SHORT:
    case INT:
    case LONG:
      parseDeclarationStatement();
      return NULL;
    case IDENTIFIER:
      return parseAssignmentStatement();
    case END:
      return parseEnd();
    default:
      fatal(RC_ERROR, "Expected valid statement, but got %s\n", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
      return NULL;
  }
}
