#include "../../include/parsing/statement.h"

extern Token GLOBAL_TOKEN;

void matchToken(TokenType type) {
  if (GLOBAL_TOKEN.type != type) {
    fatal(RC_ERROR, "Expected token type %s but got %s\n", TOKENTYPE_STRING[type], TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
  }
  scan();
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
  matchToken(INTEGER_LITERAL);

  ASTNode* result = malloc(sizeof(ASTNode*));
  result->token = (Token) {PRINT, 0};

  ASTNode* cur = malloc(sizeof(ASTNode*));
  result->left = cur;
  result->right = NULL;
  cur->token = (Token) {STAR, 0};

  int num = GLOBAL_TOKEN.val;
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
    case END:
      return parseEnd();
    default:

      fatal(RC_ERROR, "Expected valid statement (print or factorial token)\n");
      return NULL;
  }
}
