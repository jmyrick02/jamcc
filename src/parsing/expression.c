#include "../../include/parsing/expression.h"
#include "../../include/lib/logging.h"
#include "../../include/parsing/optimization.h"

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
  ASTNode* singleArgument = parseBinaryExpression();
  matchToken(RIGHT_PAREN);

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_FUNCTION_CALL(entry->type.value.function.returnType), singleArgument, NULL, NULL);
  strcpy(result->token.val.string, name);

  return result;
}

ASTNode* parseTerminalNode() {
  ASTNode* result = malloc(sizeof(ASTNode));

  if (GLOBAL_TOKEN.type == END) {
    fatal(RC_ERROR, "Expected semicolon but encountered end of file");
  } else if (GLOBAL_TOKEN.type == NUMBER_LITERAL) {
    *result = CONSTRUCTOR_ASTNODE(GLOBAL_TOKEN, NULL, NULL, NULL);
  } else if (GLOBAL_TOKEN.type == IDENTIFIER) {
    SymbolTableEntry* entry = getSymbolTableEntry(GLOBAL_TOKEN.val.string);
    if (entry == NULL)
      fatal(RC_ERROR, "Undeclared variable %s", GLOBAL_TOKEN.val.string);
    if (entry->type.type == FUNCTION_TYPE)
      return parseFunctionCall();

    *result = CONSTRUCTOR_ASTNODE(GLOBAL_TOKEN, NULL, NULL, NULL);
    result->token.valueType.type = NUMBER_TYPE;
    result->token.valueType.value.number.numType = entry->type.value.number.numType;
    result->token.valueType.value.number.pointerDepth = entry->type.value.number.pointerDepth;
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

    result->token = CONSTRUCTOR_TOKEN_AMPERSAND(entry->type.value.number.numType, entry->type.value.number.registerNum, entry->type.value.number.pointerDepth + 1);
    strcpy(result->token.val.string, identifierName); 
  } else if (GLOBAL_TOKEN.type == STAR) {
    matchToken(STAR);
    ASTNode* temp = prefixOperatorPassthrough();

    if (temp->token.type != IDENTIFIER && temp->token.type != DEREFERENCE)
      fatal(RC_ERROR, "Dereference operators must be succeeded by deference operators or variable names but found %s\n", TOKENTYPE_STRING[temp->token.type]);

    result = malloc(sizeof(ASTNode));
    *result = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_WITH_NUMBER_POINTER(DEREFERENCE, temp->token.valueType.value.number.numType, temp->token.valueType.value.number.pointerDepth - 1), temp, NULL, NULL);
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
  while (tokenType != SEMICOLON && tokenType != RIGHT_PAREN && tokenType != END && (checkPrecedence(tokenType) > prevPrecedence || (tokenType == ASSIGN && checkPrecedence(tokenType) == prevPrecedence))) {
    scan();

    ASTNode* right = prattParse(PRECEDENCE[tokenType]);
    int pointerDepthTest = left->token.valueType.value.number.pointerDepth;

    if (tokenType == ASSIGN) {
      right->isRVal = 1;
      ASTNode* temp = left;
      left = right;
      right = temp;
    } else {
      left->isRVal = 1;
      right->isRVal = 1;
    }

    // Join right subtree with current left subtree
    NumberType resultNumType;
    if (left->token.valueType.type == FUNCTION_TYPE) {
      switch (left->token.valueType.value.function.returnType) {
        case CHAR:
          resultNumType = NUM_CHAR;
          break;
        case SHORT:
          resultNumType = NUM_SHORT;
          break;
        case INT:
          resultNumType = NUM_INT;
          break;
        case LONG:
          resultNumType = NUM_LONG;
          break;
        default:
          fatal(RC_ERROR, "Invalid function (%s) tokenType result %s in binary expression\n", left->token.val.string, TOKENTYPE_STRING[left->token.valueType.value.function.returnType]);
          break;
      }
    } else {
      resultNumType = left->token.valueType.value.number.numType;
    }

    ASTNode* newLeft = malloc(sizeof(ASTNode));
    *newLeft = CONSTRUCTOR_ASTNODE(CONSTRUCTOR_TOKEN_WITH_NUMBER_POINTER(tokenType, resultNumType, pointerDepthTest), left, NULL, right);
    left = newLeft;

    tokenType = GLOBAL_TOKEN.type;
  }
  if (tokenType == END) {
    fatal(RC_ERROR, "Expected a semicolon but found end of file");
  }

  left->isRVal = 1;
  return left;
}

ASTNode* parseBinaryExpression() {
  return optimize(prattParse(-1));
}
