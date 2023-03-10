#include <stdio.h>

#include "../include/parsing/expression.h"
#include "../include/generation/generate_llvm.h"
#include "../include/generation/symboltable.h"

char* ARG_FILEPATH;

extern FILE* GLOBAL_FILE_POINTER;
extern FILE* LLVM_OUTPUT;

extern FILE* LLVM_GLOBALS_OUTPUT;

int main(int argc, char *argv[]) {
  if (argc < 2)
    fatal(RC_ERROR, "You didn't pass in an input file path");

  ARG_FILEPATH = argv[1];

  GLOBAL_FILE_POINTER = fopen(ARG_FILEPATH, "r");
  if (GLOBAL_FILE_POINTER == NULL) 
    fatal(RC_ERROR, "Failed to open file at %s", ARG_FILEPATH);

  LLVM_OUTPUT = fopen("out.ll", "w");
  if (LLVM_OUTPUT == NULL) 
    fatal(RC_ERROR, "Failed to create file out.ll");

  LLVM_GLOBALS_OUTPUT = fopen(".globals.ll", "w");
  if (LLVM_GLOBALS_OUTPUT == NULL) 
    fatal(RC_ERROR, "Failed to create file .globals.ll");

  // Initialize symbol table
  initSymbolTable(512);
  
  // Initialize scanner
  scan();

  generateLLVM();

  fclose(GLOBAL_FILE_POINTER);
}
