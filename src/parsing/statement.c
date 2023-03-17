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

ASTNode* parsePrintStatement() {
  matchToken(PRINT);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {PRINT};
  result->left = parseBinaryExpression();
  result->right = NULL;

  if (result->left->token.valueType.value.number.numType != NUM_INT)
    fatal(RC_ERROR, "Print statements only work on integers, received %s\n", NUMBERTYPE_STRING[result->left->token.valueType.value.number.numType]);
    
  return result;
}

ASTNode* parseFactorialStatement() {
  matchToken(FACTORIAL);
  Token literal = matchToken(NUMBER_LITERAL);
  if (literal.valueType.value.number.numType != NUM_INT)
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
    cur->left->token = (Token) {NUMBER_LITERAL, (TokenVal) {num}, (Type) { NUMBER_TYPE, (TypeValue) {(Number) {NUM_INT}}}};
    cur->left->left = NULL;
    cur->left->right = NULL;

    cur->right = malloc(sizeof(ASTNode));
    cur->right->token = (Token) {STAR, 0};
    cur->right->left = NULL;
    cur->right->right = NULL;

    cur = cur->right;
    num--;
  }
  cur->token = (Token) {NUMBER_LITERAL, (TokenVal) { .num = 1 }, (Type) {NUMBER_TYPE, (TypeValue) {(Number) {NUM_INT}}}};

  return result;
}

void parseDeclarationStatement() {
  Number num = matchNumType();

  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = (Type) { NUMBER_TYPE, (TypeValue) { (Number) {num.numType, num.registerNum, num.pointerDepth + 1} } };
  entry.next = NULL;

  updateSymbolTable(entry);
  generateDeclareGlobal(identifier.val.string, 0, entry.type.value.number);
}

ASTNode* parseAssignmentStatement() {
  Token identifier = matchToken(IDENTIFIER);

  SymbolTableEntry* entry = getSymbolTableEntry(identifier.val.string);
  if (entry == NULL)
    fatal(RC_ERROR, "Undefined variable %s\n", identifier.val.string);

  matchToken(ASSIGN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {ASSIGN};
  result->left = parseBinaryExpression();
  result->right = malloc(sizeof(ASTNode));
  result->right->token = (Token) {LEFTVALUE_IDENTIFIER, (TokenVal) {}, (Type) {NUMBER_TYPE, (TypeValue) {(Number) {entry->type.value.number.numType, -1, entry->type.value.number.pointerDepth}}}};
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
  result->token = (Token) {IF};
  result->left = condition;
  result->center = block;
  result->right = negativeBlock;
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

  ASTNode* condition = parseBinaryExpression();
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

ASTNode* parseReturn() {
  matchToken(RETURN);

  SymbolTableEntry* entry = getSymbolTableEntry(CUR_FUNCTION_NAME);
  if (entry == NULL)
    fatal(RC_ERROR, "Current function does not exist!");

  ASTNode* result = malloc(sizeof(ASTNode)); result->token = (Token) {RETURN, (TokenVal) {}};
  strcpy(result->token.val.string, CUR_FUNCTION_NAME);
  result->left = NULL;
  result->center = NULL;
  result->right = NULL;

  if (entry->type.value.function.returnType != VOID)
    result->left = parseBinaryExpression();
  
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
        {
          SymbolTableEntry* entry = getSymbolTableEntry(GLOBAL_TOKEN.val.string);
          if (entry == NULL)
            fatal(RC_ERROR, "Identifier %s undeclared\n", GLOBAL_TOKEN.val.string);
          if (entry->type.type == FUNCTION_TYPE) {
            root = parseFunctionCall();
          } else {
            root = parseAssignmentStatement();
          }
        }
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
        fatal(RC_ERROR, "Expected valid statement in block, but got %s\n", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
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
        left->token = (Token) {AST_GLUE};
        left->left = oldLeft;
        left->center = NULL;
        left->right = root;
      }
    }
  }
}
