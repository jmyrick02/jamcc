#include "../../include/generation/generate_llvm.h"
#include "../../include/parsing/statement.h"

FILE* LLVM_OUTPUT;
int LLVM_VIRTUAL_REGISTER_NUMBER = 0;
IntNode* LLVM_FREE_REGISTER_NUMBERS = NULL;
LLVMNode* LLVM_LOADED_REGISTERS = NULL;

extern char* ARG_FILEPATH;

#define TARGET_DATALAYOUT "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
#define TARGET_TRIPLE "x86_64-pc-linux-gnu"
#define ATTRIBUTES_0 "noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\""
#define CLANG_VERSION "clang version 15.0.7"

void pushLoadedRegister(LLVMValue vr) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->vr = vr;
  newNode->next = LLVM_LOADED_REGISTERS;

  LLVM_LOADED_REGISTERS = newNode;
}

void pushFreeRegisterNumber(int num) {
  IntNode* newNode = malloc(sizeof(IntNode));
  newNode->val = num;
  newNode->next = LLVM_FREE_REGISTER_NUMBERS;
  
  LLVM_FREE_REGISTER_NUMBERS = newNode;
}

int popFreeRegisterNumber() {
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

void generateStackAllocation(LLVMNode* head) {
  LLVMNode* cur = head;
  while (cur != NULL) {
    fprintf(LLVM_OUTPUT, "\t%%%d = alloca i32, align %d\n", cur->vr.val, cur->alignBytes);
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
  LLVMValue newVR = {VIRTUAL_REGISTER, getNextVirtualRegisterNumber()};

  fprintf(LLVM_OUTPUT, "\t%%%d = load i32, i32* %%%d\n", newVR.val, vr.val);

  pushLoadedRegister(newVR);
  return newVR;
}

LLVMValue generateStoreConstant(int constant) {
  int registerNum = popFreeRegisterNumber();
  fprintf(LLVM_OUTPUT, "\tstore i32 %d, i32* %%%d\n", constant, registerNum);

  return (LLVMValue) {VIRTUAL_REGISTER, registerNum};
}

LLVMValue generateAdd(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber(); 
  fprintf(LLVM_OUTPUT, "\t%%%d = add nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateSub(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = sub nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateMul(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = mul nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateDiv(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = udiv i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateShl(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = shl nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateAshr(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = ashr i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

  return (LLVMValue) {VIRTUAL_REGISTER, outVRNum};
}

LLVMValue generateBinaryArithmetic(Token token, LLVMValue leftVR, LLVMValue rightVR) {
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

void generatePrintInt(LLVMValue vr) {
  LLVMValue loadedVR = generateEnsureRegisterLoaded(vr);
  fprintf(LLVM_OUTPUT, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @print_int_fstring , i32 0, i32 0), i32 %%%d)\n", loadedVR.val);
  LLVM_VIRTUAL_REGISTER_NUMBER++;
}

LLVMValue generateFromAST(ASTNode* root) {
  LLVMValue leftVR;
  LLVMValue rightVR;

  if (root->left != NULL)
    leftVR = generateFromAST(root->left);
  if (root->right != NULL)
    rightVR = generateFromAST(root->right);

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
    case INTEGER_LITERAL:
      return generateStoreConstant(root->token.val.integer);
    case PRINT:
      generatePrintInt(leftVR);
      return (LLVMValue) {NONE, 0};
    default:
      fatal(RC_ERROR, "Encountered bad operand while evaluating expression");
      return leftVR; 
  }
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
  } else { // Root is an integer literal
    LLVMNode* result = malloc(sizeof(LLVMNode));
    
    int registerNumber = getNextVirtualRegisterNumber();
    result->vr = (LLVMValue) {VIRTUAL_REGISTER, registerNumber};
    result->alignBytes = 4;
    result->next = NULL;

    pushFreeRegisterNumber(registerNumber);

    return result;
  }
}

void generateLLVM() {
  generatePreamble(); 
  
  ASTNode* statementTree = parseStatement();
  while (statementTree->token.type != END) {
    generateStackAllocation(getStackEntriesFromBinaryExpression(statementTree));
    generateFromAST(statementTree);

    statementTree = parseStatement();
  }

  generatePostamble();
}
