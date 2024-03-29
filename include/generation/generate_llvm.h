#include <stdio.h>

#include "../../include/lib/logging.h"

#include "../llvm.h"
#include "../scan.h"

LLVMValue getNextLabel();

void generateDeclareGlobal(char* name, int value, Number num);

LLVMValue generateIf(ASTNode* root);

LLVMValue generateWhile(ASTNode* root);

void generateLLVM();
