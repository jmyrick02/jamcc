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
  result->left = parseBinaryExpression();
  result->right = NULL;

  matchToken(SEMICOLON);

  return result;
}

ASTNode* parseFactorialStatement() {
  matchToken(FACTORIAL);
  Token literal = matchToken(INTEGER_LITERAL);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {PRINT, 0};

  ASTNode* cur = malloc(sizeof(ASTNode));
  result->left = cur;
  result->right = NULL;
  cur->token = (Token) {STAR, 0};

  int num = literal.val.integer;
  while (num > 1) {
    cur->left = malloc(sizeof(ASTNode));
    cur->left->token = (Token) {INTEGER_LITERAL, num};
    cur->left->left = NULL;
    cur->left->right = NULL;

    cur->right = malloc(sizeof(ASTNode));
    cur->right->token = (Token) {STAR, 0};
    cur->right->left = NULL;
    cur->right->right = NULL;

    cur = cur->right;
    num--;
  }
  cur->token = (Token) {INTEGER_LITERAL, 1};

  matchToken(SEMICOLON);

  return result;
}

void parseDeclarationStatement() {
  matchToken(INT);
  Token identifier = matchToken(IDENTIFIER);

  updateSymbolTable(identifier.val.string, -1);
  generateDeclareGlobal(identifier.val.string, 0);

  matchToken(SEMICOLON);
}

ASTNode* parseAssignmentStatement() {
  Token identifier = matchToken(IDENTIFIER);

  if (getSymbolTableEntry(identifier.val.string) == NULL)
    fatal(RC_ERROR, "Undefined variable %s\n", identifier.val.string);

  matchToken(ASSIGN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->left = parseBinaryExpression();
  result->right = malloc(sizeof(ASTNode));
  result->right->token = (Token) {LEFTVALUE_IDENTIFIER, (TokenVal) {}};
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
    case INT:
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
