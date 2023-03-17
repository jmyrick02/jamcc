#include "../../include/parsing/expression.h"
#include "../../include/lib/logging.h"
#include "../../include/parsing/statement.h"

extern FILE* GLOBAL_FILE_POINTER;
extern Token GLOBAL_TOKEN;

NumberType* curNumType;

ASTNode* parseFunctionCall() {
  Token identifier = matchToken(IDENTIFIER);
  char* name = identifier.val.string;

  SymbolTableEntry* entry = getSymbolTableEntry(name);
  if (entry == NULL) {
    fatal(RC_ERROR, "Undeclared function %s\n", name);
  }

  matchToken(LEFT_PAREN);
  ASTNode* singleArgument = parseBinaryExpression();
  matchToken(RIGHT_PAREN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {FUNCTION_CALL, (TokenVal) {}};
  strcpy(result->token.val.string, name);
  result->left = singleArgument;
  result->center = NULL;
  result->right = NULL;

  return result;
}

ASTNode* parseTerminalNode() {
  ASTNode* result = malloc(sizeof(ASTNode));

  if (GLOBAL_TOKEN.type == END) {
    fatal(RC_ERROR, "Expected semicolon but encountered end of file");
  } else if (GLOBAL_TOKEN.type == NUMBER_LITERAL) {
    if (curNumType == NULL)
      curNumType = &GLOBAL_TOKEN.valueType.value.number.numType;

    if (GLOBAL_TOKEN.valueType.value.number.numType != *curNumType)
      fatal(RC_ERROR, "Number %ld is of type %s but expected %s\n", GLOBAL_TOKEN.val.num, NUMBERTYPE_STRING[GLOBAL_TOKEN.valueType.value.number.numType], NUMBERTYPE_STRING[*curNumType]);

    result->token = GLOBAL_TOKEN;
    result->left = NULL;
    result->center = NULL;
    result->right = NULL;
  } else if (GLOBAL_TOKEN.type == IDENTIFIER) {
    SymbolTableEntry* entry = getSymbolTableEntry(GLOBAL_TOKEN.val.string);
    if (entry == NULL)
      fatal(RC_ERROR, "Undeclared variable %s", GLOBAL_TOKEN.val.string);
   if (entry->type.type == FUNCTION_TYPE)
      return parseFunctionCall();
   if (curNumType == NULL)
      curNumType = &entry->type.value.number.numType;
   if (entry->type.value.number.numType != *curNumType)
      fatal(RC_ERROR, "Variable %s is of type %s but expected %s\n", entry->identifierName, NUMBERTYPE_STRING[entry->type.value.number.numType], NUMBERTYPE_STRING[*curNumType]); 

    result->token = GLOBAL_TOKEN;
    result->token.valueType.value.number.numType = entry->type.value.number.numType;
    result->left = NULL;
    result->center = NULL;
    result->right = NULL;
  } else {
    fatal(RC_ERROR, "Expected terminal token but encountered %s", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
  }

  scan();
  return result;
}

ASTNode* prefixOperatorPassthrough() {
  ASTNode* result;

  if (GLOBAL_TOKEN.type == AMPERSAND) {
    matchToken(AMPERSAND);
    result = prefixOperatorPassthrough();

    if (result->token.type != IDENTIFIER)
      fatal(RC_ERROR, "Ampersand operator must be succeeded by variable name but found %s\n", TOKENTYPE_STRING[result->token.type]);

    char identifierName[MAX_IDENTIFIER_LENGTH + 1]; 
    strcpy(identifierName, result->token.val.string);
    SymbolTableEntry* entry = getSymbolTableEntry(identifierName);
    if (entry == NULL)
      fatal(RC_ERROR, "Attempted to get the address of an undeclared variable %s\n", result->token.val.string);

    result->token = (Token) {AMPERSAND, (TokenVal) {}, (Type) {NUMBER_TYPE, (TypeValue) {(Number) {entry->type.value.number.numType, entry->type.value.number.registerNum, entry->type.value.number.pointerDepth + 1}} } };
    strcpy(result->token.val.string, identifierName); 
  } else if (GLOBAL_TOKEN.type == STAR) {
    matchToken(STAR);
    ASTNode* temp = prefixOperatorPassthrough();

    if (temp->token.type != IDENTIFIER && temp->token.type != DEREFERENCE)
      fatal(RC_ERROR, "Dereference operators must be succeeded by deference operators or variable names but found %s\n", TOKENTYPE_STRING[temp->token.type]);

    result = malloc(sizeof(ASTNode));
    result->left = temp;
    result->center = NULL;
    result->right = NULL;
    result->token = temp->token;
    result->token.valueType.value.number.pointerDepth--;
  } else {
    result = parseTerminalNode();
  }

  return result;
}

// Also checks for errors
int checkPrecedence(TokenType tokenType) {
  if (PRECEDENCE[tokenType] == -1) {
    fatal(RC_ERROR, "Expected operator but encountered %s", TOKENTYPE_STRING[tokenType]); 
  }

  return PRECEDENCE[tokenType];
}

ASTNode* prattParse(int prevPrecedence) {
  ASTNode* left = prefixOperatorPassthrough(); // parses terminal node with possible prefix operators 
  
  TokenType tokenType = GLOBAL_TOKEN.type;
  // TODO Parens in binary expression?
  while (tokenType != SEMICOLON && tokenType != RIGHT_PAREN && tokenType != END && checkPrecedence(tokenType) > prevPrecedence) {
    scan();

    ASTNode* right = prattParse(PRECEDENCE[tokenType]);

    // Join right subtree with current left subtree
    ASTNode* newLeft = malloc(sizeof(ASTNode));
    newLeft->token = (Token) {tokenType, (TokenVal) {0}, (Type) {NUMBER_TYPE, (TypeValue) { .number = (Number) {left->token.valueType.value.number.numType}}}};
    newLeft->left = left;
    newLeft->right = right;
    left = newLeft;

    tokenType = GLOBAL_TOKEN.type;
  }
  if (tokenType == END) {
    fatal(RC_ERROR, "Expected a semicolon but found end of file");
  }

  return left;
}

ASTNode* parseBinaryExpression() {
  curNumType = NULL;
  return prattParse(-1);
}
