#include "../../include/parsing/function_declaration.h"

extern Token GLOBAL_TOKEN;

extern char CUR_FUNCTION_NAME[MAX_IDENTIFIER_LENGTH + 1];

ASTNode* parseFunctionDeclaration() {
  TokenType returnType = GLOBAL_TOKEN.type;
  if (returnType != VOID && returnType != SHORT && returnType != INT && returnType != LONG && returnType != CHAR)
    fatal(RC_ERROR, "Invalid function return type %s\n", TOKENTYPE_STRING[returnType]);
  scan();
  
  Token identifier = GLOBAL_TOKEN;
  if (getSymbolTableEntry(identifier.val.string) != NULL && getSymbolTableEntry(identifier.val.string)->type.type == FUNCTION_TYPE)
    fatal(RC_ERROR, "Function %s already declared\n", identifier.val.string);
  scan();

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = (Type) { FUNCTION_TYPE, (TypeValue) { .function = (Function) {returnType}}};
  entry.next = NULL;

  updateSymbolTable(entry); 

  SymbolTableEntry* temp = getSymbolTableEntry(identifier.val.string);

  matchToken(LEFT_PAREN);
  matchToken(RIGHT_PAREN);

  strcpy(CUR_FUNCTION_NAME, identifier.val.string);

  ASTNode* result = malloc(sizeof(ASTNode));
  Token resultToken;
  resultToken.type = FUNCTION;
  strcpy(resultToken.val.string, identifier.val.string);
  result->token = resultToken;
  result->left = parseBlock();
  result->center = NULL;
  result->right = NULL;

  return result;
}
