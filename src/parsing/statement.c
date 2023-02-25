#include "../../include/parsing/statement.h"

extern Token GLOBAL_TOKEN;

void matchToken(TokenType type) {
  if (GLOBAL_TOKEN.type != type) {
    fatal(RC_ERROR, "Expected token type %s but got %s\n", TOKENTYPE_STRING[type], TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
  }
  scan();
}

ASTNode* parseStatement() {
  if (GLOBAL_TOKEN.type == END) {
    ASTNode* endNode = malloc(sizeof(ASTNode));
    endNode->token = (Token) {END, 0};
    return endNode;
  }
  ASTNode* parsedBinaryExpression;
  if (GLOBAL_TOKEN.type == PRINT) {
    scan();
    parsedBinaryExpression = parseBinaryExpression();
  } else if (GLOBAL_TOKEN.type == FACTORIAL) {
    parsedBinaryExpression = malloc(sizeof(ASTNode));

    scan();
    int num = GLOBAL_TOKEN.val;
    matchToken(INTEGER_LITERAL);

    if (num == 0 || num == 1) {
      parsedBinaryExpression = malloc(sizeof(ASTNode));
      parsedBinaryExpression->token = (Token) {STAR, 0};

      parsedBinaryExpression->left = malloc(sizeof(ASTNode));
      parsedBinaryExpression->left->token = (Token) {INTEGER_LITERAL, 1};
      parsedBinaryExpression->left->left = NULL;
      parsedBinaryExpression->left->right = NULL;
      
      parsedBinaryExpression->right = malloc(sizeof(ASTNode));
      parsedBinaryExpression->right->token = (Token) {INTEGER_LITERAL, 1};
      parsedBinaryExpression->right->left = NULL;
      parsedBinaryExpression->right->right = NULL;
    } else {
      ASTNode* cur = parsedBinaryExpression;
      cur->token = (Token) {STAR, 0};
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
    }
  } else {
    fatal(RC_ERROR, "Expected print or factorial token\n");
  }

  matchToken(SEMICOLON);

  return parsedBinaryExpression;
}
