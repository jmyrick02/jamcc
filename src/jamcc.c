#include <stdio.h>

#include "../include/parsing/expression.h"
#include "../include/generation/generate_llvm.h"
#include "../include/generation/symboltable.h"

char* ARG_FILEPATH;
int ARG_OPTIMIZATION;

extern FILE* GLOBAL_FILE_POINTER;
extern FILE* LLVM_OUTPUT;

extern FILE* LLVM_GLOBALS_OUTPUT;

int main(int argc, char *argv[]) {
  if (argc < 2)
    fatal(RC_ERROR, "You didn't pass in an input file path and an integer optimization level");

  ARG_FILEPATH = argv[1];
  if (argc >= 3)
    ARG_OPTIMIZATION = atoi(argv[2]);
  else
    ARG_OPTIMIZATION = 0;

  GLOBAL_FILE_POINTER = fopen(ARG_FILEPATH, "r");
  if (GLOBAL_FILE_POINTER == NULL) 
    fatal(RC_ERROR, "Failed to open file at %s", ARG_FILEPATH);

  LLVM_OUTPUT = fopen("out.ll", "w");
  if (LLVM_OUTPUT == NULL) 
    fatal(RC_ERROR, "Failed to create file out.ll");

  LLVM_GLOBALS_OUTPUT = fopen(".globals.ll", "w");
  if (LLVM_GLOBALS_OUTPUT == NULL) 
    fatal(RC_ERROR, "Failed to create file .globals.ll");

  initTables();
  // "Standard library" (currently just printint)
  ArgumentNode *arg = malloc(sizeof(ArgumentNode));
  *arg = CONSTRUCTOR_ARGUMENT_NODE(NUM_INT, NULL);
  strcpy(arg->name, "value");
  SymbolTableEntry printintEntry = CONSTRUCTOR_SYMBOL_TABLE_ENTRY(CONSTRUCTOR_FUNCTION_TYPE(INT, arg), NULL, CONSTRUCTOR_LLVMVALUE_NONE);
  strcpy(printintEntry.identifierName, "printint");
  addGlobal(printintEntry);
  
  // Initialize scanner
  scan();

  generateLLVM();

  fclose(GLOBAL_FILE_POINTER);
}
