#include "../../include/parsing/expression.h"
#include "../../include/lib/logging.h"
#include "../../include/parsing/statement.h"

extern FILE* GLOBAL_FILE_POINTER;
extern Token GLOBAL_TOKEN;

ASTNode* parseFunctionCall() {
  Token identifier = matchToken(IDENTIFIER);
  char* name = identifier.val.string;

  SymbolTableEntry* entry = getSymbolTableEntry(name);
  if (entry == NULL) {
    fatal(RC_ERROR, "Undeclared function %s\n", name);
  }

  matchToken(LEFT_PAREN);
  ASTNode* singleArgument = parseBinaryExpression(GLOBAL_TOKEN.valueType.value.number.numType);
  matchToken(RIGHT_PAREN);

  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {FUNCTION_CALL, (TokenVal) {}};
  strcpy(result->token.val.string, name);
  result->left = singleArgument;
  result->center = NULL;
  result->right = NULL;

  return result;
}

// Precondition: terminal node token is scanned
ASTNode* parseTerminalNode(NumberType numType) {
  ASTNode* result = malloc(sizeof(ASTNode));

  if (GLOBAL_TOKEN.type == END) {
    fatal(RC_ERROR, "Expected semicolon but encountered end of file");
  } else if (GLOBAL_TOKEN.type == NUMBER_LITERAL) {
    if (GLOBAL_TOKEN.valueType.value.number.numType != numType)
      fatal(RC_ERROR, "Number %ld is of type %s but expected %s\n", GLOBAL_TOKEN.val.num, NUMBERTYPE_STRING[GLOBAL_TOKEN.valueType.value.number.numType], NUMBERTYPE_STRING[numType]);

    result->token = GLOBAL_TOKEN;
    result->left = NULL;
    result->right = NULL;
  } else if (GLOBAL_TOKEN.type == IDENTIFIER) {
    SymbolTableEntry* entry = getSymbolTableEntry(GLOBAL_TOKEN.val.string);
    if (entry == NULL)
      fatal(RC_ERROR, "Undeclared variable %s", GLOBAL_TOKEN.val.string);
   if (entry->type.type == FUNCTION_TYPE)
      return parseFunctionCall();
   if (entry->type.value.number.numType != numType)
      fatal(RC_ERROR, "Variable %s is of type %s but expected %s\n", entry->identifierName, NUMBERTYPE_STRING[entry->type.value.number.numType], NUMBERTYPE_STRING[numType]); 

    result->token = GLOBAL_TOKEN;
    result->left = NULL;
    result->right = NULL;
  } else {
    fatal(RC_ERROR, "Expected terminal token but encountered %s", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
  }

  scan();
  return result;
}

// Also checks for errors
int checkPrecedence(TokenType tokenType) {
  if (PRECEDENCE[tokenType] == -1) {
    fatal(RC_ERROR, "Expected operator but encountered %s", TOKENTYPE_STRING[tokenType]); 
  }

  return PRECEDENCE[tokenType];
}

ASTNode* prattParse(int prevPrecedence, NumberType numType) {
  ASTNode* left = parseTerminalNode(numType);
  
  TokenType tokenType = GLOBAL_TOKEN.type;
  // TODO Parens in binary expression?
  while (tokenType != SEMICOLON && tokenType != RIGHT_PAREN && tokenType != END && checkPrecedence(tokenType) > prevPrecedence) {
    scan();

    ASTNode* right = prattParse(PRECEDENCE[tokenType], numType);

    // Join right subtree with current left subtree
    ASTNode* newLeft = malloc(sizeof(ASTNode));
    newLeft->token = (Token) {tokenType, (TokenVal) {0}, (Type) {NUMBER_TYPE, (TypeValue) { .number = (Number) {numType}}}};
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

ASTNode* parseBinaryExpression(NumberType numType) {
  return prattParse(-1, numType);
}
