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
  result->token = (Token) {PRINT};
  result->left = parseBinaryExpression(NUM_INT);
  result->right = NULL;

  if (result->left->token.valueType.number.numType != NUM_INT)
    fatal(RC_ERROR, "Print statements only work on integers, received %s\n", NUMBERTYPE_STRING[result->left->token.valueType.number.numType]);

  return result;
}

ASTNode* parseFactorialStatement() {
  matchToken(FACTORIAL);
  Token literal = matchToken(NUMBER_LITERAL);
  if (literal.valueType.number.numType != NUM_INT)
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

  return result;
}

void parseDeclarationStatement() {
  Token cur = GLOBAL_TOKEN;
  if (cur.type != SHORT && cur.type != INT && cur.type != LONG && cur.type != CHAR)
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

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = (Type) { .number = (Number) {type, -1}};
  entry.next = NULL;

  updateSymbolTable(entry);
  generateDeclareGlobal(identifier.val.string, 0, type);
}

ASTNode* parseAssignmentStatement() {
  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry* entry = getSymbolTableEntry(identifier.val.string);
  if (entry == NULL)
    fatal(RC_ERROR, "Undefined variable %s\n", identifier.val.string);

  matchToken(ASSIGN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {ASSIGN};
  result->left = parseBinaryExpression(GLOBAL_TOKEN.valueType.number.numType);
  result->right = malloc(sizeof(ASTNode));
  result->right->token = (Token) {LEFTVALUE_IDENTIFIER, (TokenVal) {}, entry->type.number.numType};
  strcpy(result->right->token.val.string, identifier.val.string);

  return result;
}

ASTNode* parseIf() {
  matchToken(IF);
  matchToken(LEFT_PAREN);

  ASTNode* condition = parseBinaryExpression(GLOBAL_TOKEN.valueType.number.numType);
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
  result->token = (Token) {IF};
  result->left = condition;
  result->center = block;
  result->right = negativeBlock;
  return result;
}

ASTNode* parseWhile() {
  matchToken(WHILE);
  matchToken(LEFT_PAREN);

  ASTNode* condition = parseBinaryExpression(GLOBAL_TOKEN.valueType.number.numType);
  if (condition->token.type != EQ && condition->token.type != NEQ && condition->token.type != LT && condition->token.type != LEQ && condition->token.type != GT && condition->token.type != GEQ)
    fatal(RC_ERROR, "While statements currently require a conditional operand, not %s\n", TOKENTYPE_STRING[condition->token.type]); 
  
  matchToken(RIGHT_PAREN);

  ASTNode* block = parseBlock();
  
  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {WHILE};
  result->left = condition;
  result->center = NULL;
  result->right = block;
  return result;
}

ASTNode* parseFor() {
  matchToken(FOR);
  matchToken(LEFT_PAREN);

  ASTNode* preamble = parseAssignmentStatement();
  matchToken(SEMICOLON);

  ASTNode* condition = parseBinaryExpression(GLOBAL_TOKEN.valueType.number.numType);
  if (condition->token.type != EQ && condition->token.type != NEQ && condition->token.type != LT && condition->token.type != LEQ && condition->token.type != GT && condition->token.type != GEQ)
    fatal(RC_ERROR, "For statements currently require a conditional operand, not %s\n", TOKENTYPE_STRING[condition->token.type]); 
  matchToken(SEMICOLON);

  ASTNode* postamble = malloc(sizeof(ASTNode));
  postamble->token = (Token) {AST_GLUE};
  
  // Add label to for postamble
  postamble->left = malloc(sizeof(ASTNode));
  int labelNum = getNextLabel().val;
  postamble->left->token = (Token) {LABEL_TOKEN, labelNum};
  postamble->left->left = NULL;
  postamble->left->center = NULL;
  postamble->left->right = NULL;
  
  postamble->center = NULL;
  postamble->right = parseAssignmentStatement();
  matchToken(RIGHT_PAREN);

  ASTNode* block = parseBlock();

  ASTNode* glue = malloc(sizeof(ASTNode));
  glue->token = (Token) {AST_GLUE};
  glue->left = block;
  glue->center = NULL;
  glue->right = postamble;

  ASTNode* whileNode = malloc(sizeof(ASTNode));
  whileNode->token = (Token) {WHILE};
  whileNode->left = condition;
  whileNode->center = NULL;
  whileNode->right = glue;

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {AST_GLUE};
  result->left = preamble;
  result->center = NULL;
  result->right = whileNode;

  return result;
}

ASTNode* parseBreak() {
  matchToken(BREAK);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {BREAK};
  result->left = NULL;
  result->center = NULL;
  result->right = NULL;

  return result;
}

ASTNode* parseContinue() {
  matchToken(CONTINUE);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {CONTINUE};
  result->left = NULL;
  result->center = NULL;
  result->right = NULL;

  return result;
}

ASTNode* parseBlock() {
  ASTNode* root = malloc(sizeof(ASTNode));
  root->token = (Token) {UNKNOWN_TOKEN};
  root->left = NULL;
  root->center = NULL;
  root->right = NULL;

  ASTNode* left = NULL;

  matchToken(LEFT_BRACE);

  while (1) {
    int matchSemicolon = 1;
    int returnLeft = 0;

    switch (GLOBAL_TOKEN.type) {
      case PRINT:
        root = parsePrintStatement();
        break;
      case FACTORIAL:
        root = parseFactorialStatement();
        break;
      case SHORT:
      case INT:
      case LONG:
      case CHAR:
        parseDeclarationStatement();
        root = malloc(sizeof(ASTNode));
        root->token.type = UNKNOWN_TOKEN;
        root->left = NULL;
        root->center = NULL;
        root->right = NULL;
        break;
      case IDENTIFIER:
        root = parseAssignmentStatement();
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
      default:
        fatal(RC_ERROR, "Expected valid statement in block, but got %s\n", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
        break;
    }

    if (returnLeft) {
      return left;
    }

    if (matchSemicolon)
      matchToken(SEMICOLON);

    if (root->token.type != UNKNOWN_TOKEN) {
      if (left == NULL) {
        left = root;
      } else {
        ASTNode* oldLeft = left;
        left = malloc(sizeof(ASTNode));
        left->token = (Token) {AST_GLUE};
        left->left = oldLeft;
        left->center = NULL;
        left->right = root;
      }
    }
  }
}
