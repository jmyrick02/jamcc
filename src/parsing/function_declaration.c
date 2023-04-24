#include "../../include/parsing/function_declaration.h"

extern Token GLOBAL_TOKEN;

extern char CUR_FUNCTION_NAME[MAX_IDENTIFIER_LENGTH + 1];

ASTNode* parseFunctionDeclaration() {
  TokenType returnType = GLOBAL_TOKEN.type;
  if (returnType != VOID && returnType != SHORT && returnType != INT && returnType != LONG && returnType != CHAR)
    fatal(RC_ERROR, "Invalid function return type %s\n", TOKENTYPE_STRING[returnType]);
  scan();
  
  Token identifier = GLOBAL_TOKEN;
  if (getGlobal(identifier.val.string) != NULL && getGlobal(identifier.val.string)->type.type == FUNCTION_TYPE)
    fatal(RC_ERROR, "Function %s already declared\n", identifier.val.string);
  scan();

  matchToken(LEFT_PAREN);

  // Parse arguments
  ArgumentNode *args = NULL;
  ArgumentNode *cur = args;

  while (GLOBAL_TOKEN.type != RIGHT_PAREN) {
    if (args != NULL)
      matchToken(COMMA);
    
    Number argType = matchNumType();
    Token argIdentifier = matchToken(IDENTIFIER);

    ArgumentNode* newNode = malloc(sizeof(ArgumentNode));
    *newNode = CONSTRUCTOR_ARGUMENT_NODE(argType.numType, NULL);
    strcpy(newNode->name, argIdentifier.val.string);

    if (cur == NULL) {
      cur = newNode;
      args = newNode;
    } else {
      cur->next = newNode;
    }

    // Add argument to function's local symbol table
    SymbolTableEntry entry = CONSTRUCTOR_SYMBOL_TABLE_ENTRY(CONSTRUCTOR_TYPE_FROM_NUMBER(argType), NULL, CONSTRUCTOR_LLVMVALUE_VR(-1, CONSTRUCTOR_TYPE_FROM_NUMBER(argType)));
    strcpy(entry.identifierName, argIdentifier.val.string);
    entry.latestLLVMValue.name = malloc(sizeof(char) * strlen(argIdentifier.val.string));
    strcpy(entry.latestLLVMValue.name, argIdentifier.val.string);
    addToTables(entry);
  }
  matchToken(RIGHT_PAREN);

  strcpy(CUR_FUNCTION_NAME, identifier.val.string);

  Token resultToken;
  resultToken.type = FUNCTION;
  resultToken.valueType = CONSTRUCTOR_FUNCTION_TYPE(returnType, args); 
  strcpy(resultToken.val.string, identifier.val.string);

  SymbolTableEntry entry;
  strcpy(entry.identifierName, identifier.val.string);
  entry.type = CONSTRUCTOR_FUNCTION_TYPE(returnType, args);
  entry.next = NULL;

  addGlobal(entry); 

  ASTNode* result = malloc(sizeof(ASTNode));
  *result = CONSTRUCTOR_ASTNODE(resultToken, parseBlock(), NULL, NULL);
  
  return result;
}
