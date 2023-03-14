#include "../../include/parsing/function_declaration.h"

extern Token GLOBAL_TOKEN;

ASTNode* parseFunctionDeclaration() {
  TokenType returnType = GLOBAL_TOKEN.type;
  if (returnType != VOID && returnType != INT && returnType != CHAR)
    fatal(RC_ERROR, "Invalid function return type %s\n", TOKENTYPE_STRING[returnType]);
  scan();
  
  Token identifier = GLOBAL_TOKEN;
  if (getSymbolTableEntry(identifier.val.string) != NULL)
    fatal(RC_ERROR, "Function %s already declared\n", identifier.val.string);
  scan();

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = (Type) { .function = (Function) {returnType}};
  entry.next = NULL;

  updateSymbolTable(entry); 

  matchToken(LEFT_PAREN);
  matchToken(RIGHT_PAREN);

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
