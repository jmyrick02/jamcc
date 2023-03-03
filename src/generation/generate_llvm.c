#include "../../include/generation/generate_llvm.h"
#include "../../include/parsing/statement.h"

FILE* LLVM_OUTPUT;
int LLVM_VIRTUAL_REGISTER_NUMBER = 0;
LLVMNode* LLVM_FREE_REGISTER_NUMBERS = NULL;
LLVMNode* LLVM_LOADED_REGISTERS = NULL;

FILE* LLVM_GLOBALS_OUTPUT;

extern char* ARG_FILEPATH;

#define TARGET_DATALAYOUT "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
#define TARGET_TRIPLE "x86_64-pc-linux-gnu"
#define ATTRIBUTES_0 "noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\""
#define CLANG_VERSION "clang version 15.0.7"

#define LLVM_GLOBALS_INJECTION_IDENTIFIER "<JAMCC GLOBALS PLACEHOLDER - If you see this, an issue with jamcc occurred>\n"

void pushLoadedRegister(LLVMValue vr) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->vr = vr;
  newNode->next = LLVM_LOADED_REGISTERS;

  LLVM_LOADED_REGISTERS = newNode;
}

void pushFreeRegister(LLVMValue vr) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->val = num;
  newNode->next = LLVM_FREE_REGISTERS;
  
  LLVM_FREE_REGISTERS = newNode;
}

int popFreeRegister(NumberType numType) {
  LLVMNode* head = LLVM_FREE_REGISTERS;
  while (head != NULL) {
    if (head->

    head = head->next;
  }

  int result = LLVM_FREE_REGISTER_NUMBERS->val;
  IntNode* old = LLVM_FREE_REGISTER_NUMBERS;
  LLVM_FREE_REGISTER_NUMBERS = old->next;
  free(old);
  return result;
}

int getNextVirtualRegisterNumber() {
  return ++LLVM_VIRTUAL_REGISTER_NUMBER;
}

void generatePreamble() {
  fprintf(LLVM_OUTPUT, "; ModuleID = '%s'\n", ARG_FILEPATH);
  fprintf(LLVM_OUTPUT, "source_filename = \"%s\"\n", ARG_FILEPATH);
  fprintf(LLVM_OUTPUT, "target datalayout = \"%s\"\n", TARGET_DATALAYOUT);
  fprintf(LLVM_OUTPUT, "target triple = \"%s\"\n", TARGET_TRIPLE);
  fprintf(LLVM_OUTPUT, "\n");
  fprintf(LLVM_OUTPUT, "@print_int_fstring = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\", align 1\n");
  fprintf(LLVM_OUTPUT, "\n");

  fprintf(LLVM_OUTPUT, LLVM_GLOBALS_INJECTION_IDENTIFIER); 
  fprintf(LLVM_OUTPUT, "\n");

  fprintf(LLVM_OUTPUT, "; Function Attrs: noinline nounwind optnone uwtable\n");
  fprintf(LLVM_OUTPUT, "define dso_local i32 @main() #0 {\n");
}

void generatePostamble() {
  fprintf(LLVM_OUTPUT, "\tret i32 0\n");
  fprintf(LLVM_OUTPUT, "}\n");
  fprintf(LLVM_OUTPUT, "\n");
  fprintf(LLVM_OUTPUT, "declare i32 @printf(i8*, ...) #1\n"); 
  fprintf(LLVM_OUTPUT, "attributes #0 = { %s }\n", ATTRIBUTES_0);
  fprintf(LLVM_OUTPUT, "\n");
  fprintf(LLVM_OUTPUT, "!llvm.module.flags = !{!0, !1, !2}");
  fprintf(LLVM_OUTPUT, "!llvm.ident = !{!3}\n");
  fprintf(LLVM_OUTPUT, "\n");
  fprintf(LLVM_OUTPUT, "!0 = !{i32 1, !\"wchar_size\", i32 4}\n");
  fprintf(LLVM_OUTPUT, "!1 = !{i32 7, !\"uwtable\", i32 2}\n");
  fprintf(LLVM_OUTPUT, "!2 = !{i32 7, !\"frame-pointer\", i32 2}\n");
  fprintf(LLVM_OUTPUT, "!3 = !{!\"%s\"}\n", CLANG_VERSION);
}

LLVMValue generateIntResize(LLVMValue vr, NumberType newWidth) {
  int outVR = getNextVirtualRegisterNumber();
  if (NUMBERTYPE_SIZE[newWidth] > NUMBERTYPE_SIZE[vr.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = zext i%d %%%d to i%d\n", outVR, NUMBERTYPE_SIZE[vr.numType], vr.val, NUMBERTYPE_SIZE[newWidth]);
  } else if (NUMBERTYPE_SIZE[newWidth] < NUMBERTYPE_SIZE[vr.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = trunc i%d %%%d to i%d\n", outVR, NUMBERTYPE_SIZE[vr.numType], vr.val, NUMBERTYPE_SIZE[newWidth]);
  }
  LLVMValue result = (LLVMValue) {VIRTUAL_REGISTER, outVR, newWidth};
  pushLoadedRegister(result);
  
  return result;
}

void generateStackAllocation(LLVMNode* head) {
  LLVMNode* cur = head;
  while (cur != NULL) {
    fprintf(LLVM_OUTPUT, "\t%%%d = alloca %s, align %d\n", cur->vr.val, NUMBERTYPE_LLVM[cur->vr.numType], cur->alignBytes);
    cur = cur->next;
  }
}

LLVMValue generateEnsureRegisterLoaded(LLVMValue vr) {
  // Check if register is loaded
  LLVMNode* cur = LLVM_LOADED_REGISTERS;
  while (cur != NULL) {
    if (cur->vr.val == vr.val) { // Register is loaded
      return vr;
    }
    cur = cur->next;
  }

  // Load the register since it's unloaded
  LLVMValue newVR = {VIRTUAL_REGISTER, getNextVirtualRegisterNumber(), vr.numType};

  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s* %%%d\n", newVR.val, NUMBERTYPE_LLVM[vr.numType], NUMBERTYPE_LLVM[vr.numType], vr.val);

  pushLoadedRegister(newVR);
  return newVR;
}

LLVMValue generateStoreConstant(long constant, NumberType type) {
  int registerNum = popFreeRegisterNumber();
  fprintf(LLVM_OUTPUT, "\tstore %s %ld, %s* %%%d\n", NUMBERTYPE_LLVM[type], constant, NUMBERTYPE_LLVM[type], registerNum);

  return (LLVMValue) {VIRTUAL_REGISTER, registerNum, type};
}

LLVMValue generateAdd(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber(); 
  fprintf(LLVM_OUTPUT, "\t%%%d = add nsw %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateSub(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = sub nsw %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateMul(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = mul nsw %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateDiv(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = udiv %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateShl(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = shl nsw %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateAshr(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = ashr %s %%%d, %%%d\n", outVRNum, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum, leftVR.numType};
}

LLVMValue generateBinaryArithmetic(Token token, LLVMValue leftVR, LLVMValue rightVR) {
  // Check for size incompatibilities
  if (NUMBERTYPE_SIZE[leftVR.numType] < NUMBERTYPE_SIZE[rightVR.numType]) {
    leftVR = generateIntResize(leftVR, rightVR.numType); // Extend leftVR to rightVR's num type
  } else if (NUMBERTYPE_SIZE[leftVR.numType] > NUMBERTYPE_SIZE[rightVR.numType]) {
    rightVR = generateIntResize(rightVR, leftVR.numType); // Extend rightVR to leftVR's num type
  }

  LLVMValue result;
  switch (token.type) {
    case PLUS:
      result = generateAdd(leftVR, rightVR);
      break;
    case MINUS:
      result = generateSub(leftVR, rightVR);
      break;
    case STAR:
      result = generateMul(leftVR, rightVR);
      break;
    case SLASH:
      result = generateDiv(leftVR, rightVR);
      break;
    case BITSHIFT_LEFT:
      result = generateShl(leftVR, rightVR);
      break;
    case BITSHIFT_RIGHT:
      result = generateAshr(leftVR, rightVR);
      break;
    default:
      fatal(RC_ERROR, "Expected binary arithmetic token but received '%s'", TOKENTYPE_STRING[token.type]);
      break;
  }

  pushLoadedRegister(result);
  return result;
}

LLVMValue generateLoadGlobal(char* string) {
  SymbolTableEntry* globalEntry = getSymbolTableEntry(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s\n", string);

  int outVR = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s* @%s\n", outVR, NUMBERTYPE_LLVM[globalEntry->numType], NUMBERTYPE_LLVM[globalEntry->numType], string);
  LLVMValue result = (LLVMValue) {VIRTUAL_REGISTER, outVR, globalEntry->numType};
  pushLoadedRegister(result);
  return result;
}

void generateStoreGlobal(char* string, LLVMValue rvalueVR) {
  SymbolTableEntry* globalEntry = getSymbolTableEntry(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s\n", string);

  LLVMValue outVR = generateEnsureRegisterLoaded((LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType});
  if (outVR.numType != globalEntry->numType)
    outVR = generateIntResize(outVR, globalEntry->numType);
  fprintf(LLVM_OUTPUT, "\tstore %s %%%d, %s* @%s\n", NUMBERTYPE_LLVM[globalEntry->numType], outVR.val, NUMBERTYPE_LLVM[globalEntry->numType], string);
}

void generatePrintInt(LLVMValue vr) { // TODO how to print nonints?
  LLVM_VIRTUAL_REGISTER_NUMBER++;
  fprintf(LLVM_OUTPUT, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @print_int_fstring , i32 0, i32 0), i32 %%%d)\n", vr.val);
}

LLVMValue generateComparison(Token comparison, LLVMValue leftVR, LLVMValue rightVR) {
  // Check for size incompatibilities
  if (NUMBERTYPE_SIZE[leftVR.numType] < NUMBERTYPE_SIZE[rightVR.numType]) {
    leftVR = generateIntResize(leftVR, rightVR.numType); // Extend leftVR to rightVR's num type
  } else if (NUMBERTYPE_SIZE[leftVR.numType] > NUMBERTYPE_SIZE[rightVR.numType]) {
    rightVR = generateIntResize(rightVR, leftVR.numType); // Extend rightVR to leftVR's num type
  }

  int boolOutVR = getNextVirtualRegisterNumber();

  char* op;
  switch (comparison.type) {
    case EQ:
      op = "eq";
      break;
    case NEQ:
      op = "ne";
      break;
    case LT:
      op = "slt";
      break;
    case LEQ:
      op = "sle";
      break;
    case GT:
      op = "sgt";
      break;
    case GEQ:
      op = "sge";
      break;
    default:
      fatal(RC_ERROR, "LLVM Comparison received non-comparison token %s\n", TOKENTYPE_STRING[comparison.type]);
      break;
  }

  fprintf(LLVM_OUTPUT, "\t%%%d = icmp %s %s %%%d, %%%d\n", boolOutVR, op, NUMBERTYPE_LLVM[leftVR.numType], leftVR.val, rightVR.val);

  // Extend bool (i1) to int (i32)
  return generateIntResize((LLVMValue) {VIRTUAL_REGISTER, boolOutVR, NUM_BOOL}, NUM_INT);
}

LLVMValue generateFromAST(ASTNode* root, LLVMValue rvalueVR) {
  LLVMValue leftVR;
  LLVMValue rightVR;

  if (root->left != NULL)
    leftVR = generateFromAST(root->left, (LLVMValue) {NONE, 0});
  if (root->right != NULL)
    rightVR = generateFromAST(root->right, leftVR);

  switch (root->token.type) {
    case PLUS:
    case MINUS:
    case STAR:
    case SLASH:
    case BITSHIFT_LEFT:
    case BITSHIFT_RIGHT:
      leftVR = generateEnsureRegisterLoaded(leftVR);
      rightVR = generateEnsureRegisterLoaded(rightVR);
      return generateBinaryArithmetic(root->token, leftVR, rightVR);
    case NUMBER_LITERAL:
        return generateStoreConstant(root->token.val.num, root->token.numType);
    case PRINT:
      generatePrintInt(leftVR);
      return (LLVMValue) {NONE, 0};
    case IDENTIFIER:
      return generateLoadGlobal(root->token.val.string);
    case LEFTVALUE_IDENTIFIER:
      generateStoreGlobal(root->token.val.string, rvalueVR);
      return (LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType}; // TODO numType
    case ASSIGN:
      return (LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType};
    default:
      fatal(RC_ERROR, "Encountered bad operand while evaluating expression: %s", TOKENTYPE_STRING[root->token.type]);
      return leftVR; 
  }
}

void generateDeclareGlobal(char* name, int value, NumberType numType) {
  fprintf(LLVM_GLOBALS_OUTPUT, "@%s = global %s %d\n", name, NUMBERTYPE_LLVM[numType], value);
}

LLVMNode* getStackEntriesFromBinaryExpression(ASTNode* root) {
  if (root->left != NULL || root->right != NULL) {
    LLVMNode* left = NULL;
    LLVMNode* right = NULL;
    if (root->left != NULL) {
      left = getStackEntriesFromBinaryExpression(root->left);
    }
    if (root->right != NULL) {
      right = getStackEntriesFromBinaryExpression(root->right);
    }

    if (left != NULL) {
      LLVMNode* cur = left;
      while (cur->next != NULL) {
        cur = cur->next;
      }
      cur->next = right;
    } else {
      left = right;
    }

    return left;
  } else { // Root is a terminal 
    LLVMNode* result = malloc(sizeof(LLVMNode));

    int registerNumber = getNextVirtualRegisterNumber();

    NumberType numType;
    if (root->token.type == NUMBER_LITERAL) {
      numType = root->token.numType;
    } else {
      SymbolTableEntry* entry = getSymbolTableEntry(root->token.val.string);
      if (entry == NULL)
        fatal(RC_ERROR, "Undeclared variable %s\n", root->token.val.string);

      numType = entry->numType;
    }

    result->vr = (LLVMValue) {VIRTUAL_REGISTER, registerNumber, numType}; // TODO change type
    result->alignBytes = 4; // TODO change alignment
    result->next = NULL;

    pushFreeRegisterNumber(registerNumber);

    return result;
  }
}

// TODO scuffed
void injectGlobals() {
  LLVM_OUTPUT = fopen("out.ll", "r");
  LLVM_GLOBALS_OUTPUT = fopen(".globals.ll", "r");
  FILE* temp = fopen(".temp.ll", "w");

  char curLine[4096];

  while (fgets(curLine, 4096, LLVM_OUTPUT) != NULL) {
    if (strcmp(curLine, LLVM_GLOBALS_INJECTION_IDENTIFIER) == 0) {
      char curLineGlobals[4096];
      while (fgets(curLineGlobals, 4096, LLVM_GLOBALS_OUTPUT) != NULL) {
        fprintf(temp, "%s", curLineGlobals);
      }
    } else {
      fprintf(temp, "%s", curLine);
    }
  }

  fclose(LLVM_OUTPUT);
  fclose(temp);
  temp = fopen(".temp.ll", "r");
  LLVM_OUTPUT = fopen("out.ll", "w");

  while (fgets(curLine, 4096, temp) != NULL) {
    fprintf(LLVM_OUTPUT, "%s", curLine);
  }

  fclose(temp);
  fclose(LLVM_GLOBALS_OUTPUT);

  if (remove(".temp.ll") != 0)
    fatal(RC_ERROR, "Failed to delete temporary file .temp.ll");
  if (remove(".globals.ll") != 0)
    fatal(RC_ERROR, "Failed to delete temporary file .globals.ll");
}

void generateLLVM() {
  generatePreamble(); 
  
  ASTNode* statementTree = parseStatement();
  while (statementTree == NULL || statementTree->token.type != END) {
    if (statementTree == NULL) {
      statementTree = parseStatement();
      continue;
    }

    generateStackAllocation(getStackEntriesFromBinaryExpression(statementTree));
    generateFromAST(statementTree, (LLVMValue) {NONE, 0});

    statementTree = parseStatement();
  }

  generatePostamble();

  fclose(LLVM_OUTPUT);
  fclose(LLVM_GLOBALS_OUTPUT);

  injectGlobals();
}
