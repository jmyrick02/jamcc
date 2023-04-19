#include "../../include/parsing/statement.h"

extern Token GLOBAL_TOKEN;

extern char CUR_FUNCTION_NAME[MAX_IDENTIFIER_LENGTH + 1];

Token matchToken(TokenType type) {
  Token cur = GLOBAL_TOKEN;
  if (cur.type != type) {
    fatal(RC_ERROR, "Expected token type %s but got %s\n", TOKENTYPE_STRING[type], TOKENTYPE_STRING[cur.type]);
  }
  scan();
  return cur;
}

Number matchNumType() {
  Token token = GLOBAL_TOKEN;
  scan();

  NumberType numType;
  switch (token.type) {
    case CHAR:
      numType = NUM_CHAR;
      break;
    case SHORT:
      numType = NUM_SHORT;
      break;
    case INT:
      numType = NUM_INT;
      break;
    case LONG:
      numType = NUM_LONG;
      break;
    default:
      fatal(RC_ERROR, "Expected num type but received %s\n", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
  }

  int pointerDepth = 0;
  while (GLOBAL_TOKEN.type == STAR) {
    matchToken(STAR);
    pointerDepth++;
  }

  return (Number) {numType, -1, pointerDepth};
}

ASTNode *parseDeclarationStatement() {
  Number num = matchNumType();

  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = CONSTRUCTOR_NUMBER_TYPE_FULL(num.numType, num.registerNum, num.pointerDepth);
  entry.next = NULL;

  if (GLOBAL_TOKEN.type == LEFT_BRACKET) {
    matchToken(LEFT_BRACKET);
    if (GLOBAL_TOKEN.type == NUMBER_LITERAL) {
      entry.type = CONSTRUCTOR_ARRAY_TYPE(GLOBAL_TOKEN.valueType.value.number, GLOBAL_TOKEN.val.num);
      matchToken(NUMBER_LITERAL);
    } else {
      fatal(RC_ERROR, "Arrays must be declared with constant size\n");
    }
    matchToken(RIGHT_BRACKET);
  }

  addToTables(entry);
  
  Token resultToken = CONSTRUCTOR_TOKEN_EMPTY(VAR_DECL);
  strcpy(resultToken.val.string, identifier.val.string);

  ASTNode *result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(resultToken, NULL, NULL, NULL);
  return result;
}

ASTNode* parseAssignmentStatement() {
  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry* entry = getTables(identifier.val.string);
  if (entry == NULL)
    fatal(RC_ERROR, "Undefined variable %s\n", identifier.val.string);

  matchToken(ASSIGN);

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(ASSIGN), parseBinaryExpression(), NULL, malloc(sizeof(ASTNode)));
  *result->right = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_NUMBER_IDENTIFIER(entry->type.value.number.numType, entry->type.value.number.pointerDepth), NULL, NULL, NULL);
  strcpy(result->right->token.val.string, identifier.val.string);

  return result;
}

ASTNode* parseIf() {
  matchToken(IF);
  matchToken(LEFT_PAREN);

  ASTNode* condition = parseBinaryExpression();
  if (condition->token.type != EQ && condition->token.type != NEQ && condition->token.type != LT && condition->token.type != LEQ && condition->token.type != GT && condition->token.type != GEQ)
    fatal(RC_ERROR, "If statements currently require a conditional operand, not %s\n", TOKENTYPE_STRING[condition->token.type]); 
  
  matchToken(RIGHT_PAREN);

  ASTNode* block = parseBlock();
  ASTNode* negativeBlock = NULL;
  if (GLOBAL_TOKEN.type == ELSE) {
    matchToken(ELSE);
    negativeBlock = parseBlock();
  }

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(IF), condition, block, negativeBlock);
  return result;
}

ASTNode* parseWhile() {
  matchToken(WHILE);
  matchToken(LEFT_PAREN);

  ASTNode* condition = parseBinaryExpression();
  if (condition->token.type != EQ && condition->token.type != NEQ && condition->token.type != LT && condition->token.type != LEQ && condition->token.type != GT && condition->token.type != GEQ)
    fatal(RC_ERROR, "While statements currently require a conditional operand, not %s\n", TOKENTYPE_STRING[condition->token.type]); 
  
  matchToken(RIGHT_PAREN);

  ASTNode* block = parseBlock();
  ASTNode* elseBlock = NULL;
  if (GLOBAL_TOKEN.type == ELSE) {
    matchToken(ELSE);
    elseBlock = parseBlock();
  }

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(WHILE), condition, block, elseBlock);
  return result;
}

ASTNode* parseFor() {
  matchToken(FOR);
  matchToken(LEFT_PAREN);

  ASTNode* preamble = parseAssignmentStatement();
  matchToken(SEMICOLON);

  ASTNode* condition = parseBinaryExpression();
  if (condition->token.type != EQ && condition->token.type != NEQ && condition->token.type != LT && condition->token.type != LEQ && condition->token.type != GT && condition->token.type != GEQ)
    fatal(RC_ERROR, "For statements currently require a conditional operand, not %s\n", TOKENTYPE_STRING[condition->token.type]); 
  matchToken(SEMICOLON);

  ASTNode* postamble = malloc(sizeof(ASTNode));
  *postamble = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(AST_GLUE), malloc(sizeof(ASTNode)), NULL, parseAssignmentStatement());
  
  // Add label to for postamble
  *postamble->left = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_LABEL(getNextLabel().val), NULL, NULL, NULL);
  
  matchToken(RIGHT_PAREN);

  ASTNode* block = parseBlock();
  ASTNode* elseBlock = NULL;
  if (GLOBAL_TOKEN.type == ELSE) {
    matchToken(ELSE);
    elseBlock = parseBlock();
  }

  ASTNode* glue = malloc(sizeof(ASTNode));
  *glue = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(AST_GLUE), block, NULL, postamble);

  ASTNode* whileNode = malloc(sizeof(ASTNode));
  *whileNode = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(WHILE), condition, glue, elseBlock);

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(AST_GLUE), preamble, whileNode, NULL);

  return result;
}

ASTNode* parseBreak() {
  matchToken(BREAK);

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(BREAK), NULL, NULL, NULL);

  return result;
}

ASTNode* parseContinue() {
  matchToken(CONTINUE);

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(CONTINUE), NULL, NULL, NULL);

  return result;
}

ASTNode* parseReturn() {
  matchToken(RETURN);

  SymbolTableEntry* entry = getGlobal(CUR_FUNCTION_NAME);
  if (entry == NULL)
    fatal(RC_ERROR, "Current function does not exist!");

  ASTNode* result = malloc(sizeof(ASTNode)); 
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_RETURN, NULL, NULL, NULL);
  strcpy(result->token.val.string, CUR_FUNCTION_NAME);

  if (entry->type.value.function.returnType != VOID)
    result->left = parseBinaryExpression();
  
  return result;
}

ASTNode* parseBlock() {
  ASTNode* root = malloc(sizeof(ASTNode));
  *root = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(UNKNOWN_TOKEN), NULL, NULL, NULL);
  
  ASTNode* left = NULL;

  matchToken(LEFT_BRACE);

  while (1) {
    int matchSemicolon = 1;
    int returnLeft = 0;

    switch (GLOBAL_TOKEN.type) {
      case SHORT:
      case INT:
      case LONG:
      case CHAR:
        root = parseDeclarationStatement();
        break;
      case IF:
        root = parseIf();
        matchSemicolon = 0;
        break;
      case WHILE:
        root = parseWhile();
        matchSemicolon = 0;
        break;
      case FOR:
        root = parseFor();
        matchSemicolon = 0;
        break;
      case BREAK:
        root = parseBreak();
        break;
      case CONTINUE:
        root = parseContinue();
        break;
      case RIGHT_BRACE:
        matchToken(RIGHT_BRACE);
        matchSemicolon = 0;
        returnLeft = 1;
        break;
      case RETURN:
        root = parseReturn();
        break;
      default:
        root = parseBinaryExpression();
        break;
    }

    if (returnLeft) {
      if (left != NULL)
        return left;
      return root;
    }

    if (matchSemicolon)
      matchToken(SEMICOLON);

    if (root->token.type != UNKNOWN_TOKEN) {
      if (left == NULL) {
        left = root;
      } else {
        ASTNode* oldLeft = left;
        left = malloc(sizeof(ASTNode));
        *left = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_EMPTY(AST_GLUE), oldLeft, NULL, root);
      }
    }
  }
}
