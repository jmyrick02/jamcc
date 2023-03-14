#include <stdio.h>

#include "../../include/lib/logging.h"

#include "../llvm.h"
#include "../scan.h"

void generateDeclareGlobal(char* name, int value, NumberType numType);

LLVMValue generateIf(ASTNode* root);

LLVMValue generateWhile(ASTNode* root);

void generateLLVM();
