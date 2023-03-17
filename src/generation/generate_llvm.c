#include "../../include/generation/generate_llvm.h"
#include "../../include/parsing/statement.h"
#include "../../include/parsing/function_declaration.h"

FILE* LLVM_OUTPUT;
int LLVM_VIRTUAL_REGISTER_NUMBER = 0;
LLVMNode* LLVM_FREE_REGISTERS = NULL;
int LLVM_LABEL_INDEX = 0;

LLVMNode* CONTINUE_LABELS = NULL;
LLVMNode* BREAK_LABELS = NULL;

FILE* LLVM_GLOBALS_OUTPUT;

extern Token GLOBAL_TOKEN;

extern char* ARG_FILEPATH;

#define LABEL_PREFIX "L"

#define TARGET_DATALAYOUT "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
#define TARGET_TRIPLE "x86_64-pc-linux-gnu"
#define ATTRIBUTES_0 "noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\""
#define CLANG_VERSION "clang version 15.0.7"

#define LLVM_GLOBALS_INJECTION_IDENTIFIER "<JAMCC GLOBALS PLACEHOLDER - If you see this, an issue with jamcc occurred>\n"

void pushFreeRegister(LLVMValue vr) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->val = vr;
  newNode->next = LLVM_FREE_REGISTERS;
  
  LLVM_FREE_REGISTERS = newNode;
}

void pushContinueLabel(LLVMValue label) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->val = label;
  newNode->next = CONTINUE_LABELS;

  CONTINUE_LABELS = newNode;
}

void pushBreakLabel(LLVMValue label) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  newNode->val = label;
  newNode->next = BREAK_LABELS;

  BREAK_LABELS = newNode;
}

// TODO pop pointers
LLVMValue popFreeRegister(NumberType numType, int pointerDepth) {
  LLVMNode* cur = LLVM_FREE_REGISTERS;
  LLVMNode* prev = NULL;
  while (cur != NULL) {
    if (cur->val.numType == numType && cur->val.pointerDepth == pointerDepth)
      break;

    prev = cur;
    cur = cur->next;
  }
  if (cur == NULL)
    fatal(RC_ERROR, "No register of type %s\n", NUMBERTYPE_STRING[numType]);
  
  if (prev == NULL) {
    LLVM_FREE_REGISTERS = cur->next;
    return cur->val;
  }

  prev->next = cur->next;
  return cur->val;
}

LLVMValue popContinueLabel() {
  LLVMNode* head = CONTINUE_LABELS;
  CONTINUE_LABELS = CONTINUE_LABELS->next;
  LLVMValue result = head->val;
  free(head);
  return result;
}

LLVMValue popBreakLabel() {
  LLVMNode* head = BREAK_LABELS;
  BREAK_LABELS = BREAK_LABELS->next;
  LLVMValue result = head->val;
  free(head);
  return result;
}

int getNextVirtualRegisterNumber() {
  return ++LLVM_VIRTUAL_REGISTER_NUMBER;
}

LLVMValue getNextLabel() {
  return (LLVMValue) {LABEL, ++LLVM_LABEL_INDEX};
}

char* tokenTypeToLLVM(TokenType tokenType) {
  switch (tokenType) {
    case VOID:
      return "void";
    case CHAR:
      return "i8";
    case SHORT:
      return "i16";
    case INT:
      return "i32";
    case LONG:
      return "i64";
    default: 
      fatal(RC_ERROR, "Token type %s has no LLVM representation\n", TOKENTYPE_STRING[tokenType]);
      return "";
  }
}

char* numberToLLVM(Number num) {
  const char* base = NUMBERTYPE_LLVM[num.numType];
  int baseLen = strlen(base);

  int resLen = baseLen + num.pointerDepth;

  char* res = malloc(sizeof(char) * (resLen + 1));
  for (int i = 0; i < resLen; i++) {
    if (i < baseLen) { 
      res[i] = base[i];
    } else {
      res[i] = '*';
    }
  }
  res[resLen] = '\0';

  return res;
}

NumberType tokenTypeToNumberType(TokenType tokenType) {
  switch (tokenType) {
    case CHAR:
      return NUM_CHAR;
    case SHORT:
      return NUM_SHORT;
    case INT:
      return NUM_INT;
    case LONG:
      return NUM_LONG;
    default:
      fatal(RC_ERROR, "Token type %s has no associated number type\n", TOKENTYPE_STRING[tokenType]);
      return NUM_LONG;
  }
}

LLVMNode* getStackEntriesFromBinaryExpression(ASTNode* root) {
  if (root == NULL)
    return NULL;

  if (root->left != NULL || root->right != NULL) {
    LLVMNode* left = NULL;
    LLVMNode* center = NULL;
    LLVMNode* right = NULL;
    if (root->left != NULL) {
      left = getStackEntriesFromBinaryExpression(root->left);
    }
    if (root->center != NULL) {
      center = getStackEntriesFromBinaryExpression(root->center);
    }
    if (root->right != NULL) {
      right = getStackEntriesFromBinaryExpression(root->right);
    }

    // Concatenate left, center, and right
    LLVMNode* cur = NULL;
    LLVMNode* head = NULL;
    LLVMNode* tail = NULL;
    if (left != NULL) {
      head = left;
      cur = left;
      while (cur->next != NULL) {
        cur = cur->next;
      }
      tail = cur;
    }
    if (center != NULL) {
      if (head == NULL) {
        head = center;
      } else {
        tail->next = center;
      }
      cur = center;
      while (cur->next != NULL) {
        cur = cur->next;
      }
      tail = cur;
    }
    if (right != NULL) {
      if (head == NULL) {
        head = right;
      } else {
        tail->next = right;
      }
      cur = right;
      while (cur->next != NULL) {
        cur = cur->next;
      }
      tail = cur;
    }
    return head;
  } else { // Root is a terminal 
    LLVMNode* result = malloc(sizeof(LLVMNode));

    NumberType numType;
    int pointerDepth;
    if (root->token.type == NUMBER_LITERAL) {
      numType = root->token.valueType.value.number.numType;
      pointerDepth = 0;
      //pointerDepth = root->token.valueType.value.number.pointerDepth;
    } else if (root->token.type == IDENTIFIER) {
      SymbolTableEntry* entry = getSymbolTableEntry(root->token.val.string);
      if (entry == NULL)
        fatal(RC_ERROR, "Undeclared variable %s while generating stack entries\n", root->token.val.string);

      numType = entry->type.value.number.numType;
      pointerDepth = entry->type.value.number.pointerDepth;
    } else {
      return NULL;
    }

    int registerNumber = getNextVirtualRegisterNumber();
    result->val = (LLVMValue) {VIRTUAL_REGISTER, registerNumber, numType, pointerDepth};
    result->alignBytes = 4; // TODO change alignment?
    result->next = NULL;

    pushFreeRegister(result->val);

    return result;
  }
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
}

void generatePostamble() {
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

void generateFunctionPreamble(char* name) {
  SymbolTableEntry* entry = getSymbolTableEntry(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Generating undeclared function preamble\n");
  
  fprintf(LLVM_OUTPUT, "define dso_local %s @%s() #0 {\n", tokenTypeToLLVM(entry->type.value.function.returnType), name);
}

void generateFunctionPostamble(char* name) {
  SymbolTableEntry* entry = getSymbolTableEntry(name);
  if (entry == NULL)
    fatal(RC_ERROR, "ried to close undeclared function %s\n", name);

  fprintf(LLVM_OUTPUT, "\tret ");

  if (entry->type.value.function.returnType == VOID) {
    fprintf(LLVM_OUTPUT, "void\n");
  } else {
    fprintf(LLVM_OUTPUT, "%s 0\n", tokenTypeToLLVM(entry->type.value.function.returnType));
  }

  fprintf(LLVM_OUTPUT, "}\n\n");
}

LLVMValue generateIntResize(LLVMValue vr, NumberType newWidth) {
  int outVR = getNextVirtualRegisterNumber();
  if (NUMBERTYPE_SIZE[newWidth] > NUMBERTYPE_SIZE[vr.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = zext i%d %%%d to i%d\n", outVR, NUMBERTYPE_SIZE[vr.numType], vr.val, NUMBERTYPE_SIZE[newWidth]);
  } else if (NUMBERTYPE_SIZE[newWidth] < NUMBERTYPE_SIZE[vr.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = trunc i%d %%%d to i%d\n", outVR, NUMBERTYPE_SIZE[vr.numType], vr.val, NUMBERTYPE_SIZE[newWidth]);
  }
  LLVMValue result = (LLVMValue) {VIRTUAL_REGISTER, outVR, newWidth, 0};
  
  return result;
}

void generateStackAllocation(LLVMNode* head) {
  LLVMNode* cur = head;
  while (cur != NULL) {
    fprintf(LLVM_OUTPUT, "\t%%%d = alloca %s, align %d\n", cur->val.val, numberToLLVM((Number) {cur->val.numType, -1, cur->val.pointerDepth}), cur->alignBytes);
    cur = cur->next;
  }
}

LLVMValue generateEnsureRegisterLoaded(LLVMValue vr, int loadLevel) {
  // Check if register is already loaded
  if (vr.pointerDepth == loadLevel)
    return vr;

  // Load the register since it's unloaded
  LLVMValue newVR = {VIRTUAL_REGISTER, getNextVirtualRegisterNumber(), vr.numType, vr.pointerDepth - 1};

  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s %%%d\n", newVR.val, numberToLLVM((Number) {newVR.numType, -2, newVR.pointerDepth}), numberToLLVM((Number) {vr.numType, -3, vr.pointerDepth}), vr.val);

  return newVR;
}

LLVMValue generateStoreConstant(long constant, NumberType type) {
  LLVMValue vr = popFreeRegister(type, 0);
  fprintf(LLVM_OUTPUT, "\tstore %s %ld, %s* %%%d\n", NUMBERTYPE_LLVM[type], constant, NUMBERTYPE_LLVM[type], vr.val);

  vr.pointerDepth = 1;
  return generateEnsureRegisterLoaded(vr, 0);
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
    case EQ:
    case NEQ:
    case LT:
    case LEQ:
    case GT:
    case GEQ:
      result = generateComparison(token, leftVR, rightVR);
      break;
    default:
      fatal(RC_ERROR, "Expected binary arithmetic token but received '%s'", TOKENTYPE_STRING[token.type]);
      break;
  }

  return result;
}

LLVMValue generateLoadGlobal(char* string) {
  SymbolTableEntry* globalEntry = getSymbolTableEntry(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s encountered while loading global\n", string);

  int outVR = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s @%s\n", outVR, numberToLLVM((Number) {globalEntry->type.value.number.numType, globalEntry->type.value.number.registerNum, globalEntry->type.value.number.pointerDepth - 1}), numberToLLVM(globalEntry->type.value.number), string);
  LLVMValue result = (LLVMValue) {VIRTUAL_REGISTER, outVR, globalEntry->type.value.number.numType, globalEntry->type.value.number.pointerDepth - 1};
  return result;
}

void generateStoreGlobal(char* string, LLVMValue rvalueVR) {
  SymbolTableEntry* globalEntry = getSymbolTableEntry(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s encountered while storing global\n", string);

  LLVMValue outVR = generateEnsureRegisterLoaded((LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType, rvalueVR.pointerDepth}, globalEntry->type.value.number.pointerDepth - 1);
  if (outVR.numType != globalEntry->type.value.number.numType)
    outVR = generateIntResize(outVR, globalEntry->type.value.number.numType);

  fprintf(LLVM_OUTPUT, "\tstore %s %%%d, %s @%s\n", numberToLLVM((Number) {outVR.numType, -1, outVR.pointerDepth}), outVR.val, numberToLLVM(globalEntry->type.value.number), string);
}

void generateLabel(LLVMValue label) {
  if (label.type != LABEL)
    fatal(RC_ERROR, "Expected label for label\n");

  fprintf(LLVM_OUTPUT, "\t%s%d:\n", LABEL_PREFIX, label.val);
}

void generateJump(LLVMValue label) {
  if (label.type != LABEL)
    fatal(RC_ERROR, "Expected label for jump\n");

  fprintf(LLVM_OUTPUT, "\tbr label %%%s%d\n", LABEL_PREFIX, label.val);
}

void generateConditionalJump(LLVMValue conditionVR, LLVMValue trueLabel, LLVMValue falseLabel) {
  if (trueLabel.type != LABEL || falseLabel.type != LABEL)
    fatal(RC_ERROR, "Expected labels in conditional jump\n");
  
  fprintf(LLVM_OUTPUT, "\tbr %s %%%d, label %%%s%d, label %%%s%d\n", NUMBERTYPE_LLVM[conditionVR.numType], conditionVR.val, LABEL_PREFIX, trueLabel.val, LABEL_PREFIX, falseLabel.val);
}

LLVMValue generateCompareJump(Token comparison, LLVMValue leftVR, LLVMValue rightVR, LLVMValue falseLabel) {
  LLVMValue comparisonResult = generateComparison(comparison, leftVR, rightVR);
  comparisonResult = generateIntResize(comparisonResult, NUM_BOOL); 
  LLVMValue trueLabel = getNextLabel();
  generateConditionalJump(comparisonResult, trueLabel, falseLabel);
  generateLabel(trueLabel);
  return comparisonResult;
}

void generateReturn(LLVMValue returnValue, char* functionName) {
  SymbolTableEntry* entry = getSymbolTableEntry(functionName);
  if (entry == NULL)
    fatal(RC_ERROR, "Tried to return for undeclared function %s\n", functionName);

  LLVM_VIRTUAL_REGISTER_NUMBER++;
  if (entry->type.value.function.returnType == VOID) {
    fprintf(LLVM_OUTPUT, "\tret void\n");
  } else {
    returnValue = generateEnsureRegisterLoaded(returnValue, 0);
    fprintf(LLVM_OUTPUT, "\tret %s %%%d\n", NUMBERTYPE_LLVM[returnValue.numType], returnValue.val); 
  }
}

LLVMValue generateFunctionCall(char* name, LLVMValue arg) {
  LLVMValue result = (LLVMValue) {NONE};

  SymbolTableEntry* entry = getSymbolTableEntry(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Generating call for undeclared function %s\n", name);
  if (entry->type.type != FUNCTION_TYPE)
    fatal(RC_ERROR, "Calling a non-function identifier %s as a function\n", name);

  fprintf(LLVM_OUTPUT, "\t");

  if (entry->type.value.function.returnType != VOID) {
    result = (LLVMValue) {VIRTUAL_REGISTER, getNextVirtualRegisterNumber(), tokenTypeToNumberType(entry->type.value.function.returnType)};

    fprintf(LLVM_OUTPUT, "%%%d = ", result.val);
  }

  fprintf(LLVM_OUTPUT, "call %s () @%s()\n", tokenTypeToLLVM(entry->type.value.function.returnType), name);

  return result;
}

LLVMValue generateGetAddress(char* identifier) {
  SymbolTableEntry* entry = getSymbolTableEntry(identifier);
  if (entry == NULL)
    fatal(RC_ERROR, "Getting address of undeclared identifier %s\n", identifier);
  if (entry->type.type != NUMBER_TYPE)
    fatal(RC_ERROR, "Can only get the address of numbers\n");

  LLVMValue out = (LLVMValue) {VIRTUAL_REGISTER, getNextVirtualRegisterNumber(), entry->type.value.number.numType, entry->type.value.number.pointerDepth};

  LLVMNode* temp = malloc(sizeof(LLVMNode));
  temp->val = out;
  temp->alignBytes = 4;
  temp->next = NULL;
  generateStackAllocation(temp);

  out.pointerDepth++;

  fprintf(LLVM_OUTPUT, "\tstore %s @%s, %s %%%d\n", numberToLLVM(entry->type.value.number), identifier, numberToLLVM((Number) {out.numType, -1, out.pointerDepth}), out.val);
  
  return out;
}

LLVMValue generateDereference(LLVMValue value) {
  LLVMValue out = (LLVMValue) {VIRTUAL_REGISTER, getNextVirtualRegisterNumber(), value.numType, value.pointerDepth - 1};

  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s %%%d\n", out.val, numberToLLVM((Number) {out.numType, -1, out.pointerDepth}), numberToLLVM((Number) {value.numType, -1, value.pointerDepth}), value.val);

  return out;
}

void generatePrintInt(LLVMValue vr) {
  vr = generateEnsureRegisterLoaded(vr, vr.pointerDepth);

  LLVM_VIRTUAL_REGISTER_NUMBER++;
  fprintf(LLVM_OUTPUT, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @print_int_fstring , i32 0, i32 0), %s %%%d)\n", numberToLLVM((Number) {vr.numType, -1, vr.pointerDepth}), vr.val);
}

LLVMValue generateFromAST(ASTNode* root, LLVMValue rvalueVR, TokenType parentOperation) {
  if (root == NULL)
    return (LLVMValue) {NONE};

  LLVMValue leftVR;
  LLVMValue rightVR;

  if (root->token.type == IF) {
    return generateIf(root);
  } else if (root->token.type == WHILE) {
    return generateWhile(root);
  } else if (root->token.type == AST_GLUE) {
    generateFromAST(root->left, (LLVMValue) {NONE}, root->token.type);
    generateFromAST(root->center, (LLVMValue) {NONE}, root->token.type);
    generateFromAST(root->right, (LLVMValue) {NONE}, root->token.type);
    return (LLVMValue) {NONE};
  } else if (root->token.type == FUNCTION) {
    generateFunctionPreamble(root->token.val.string);
    generateStackAllocation(getStackEntriesFromBinaryExpression(root));
    generateFromAST(root->left, (LLVMValue) {NONE}, root->token.type);
    generateFunctionPostamble(root->token.val.string);
    return (LLVMValue) {NONE};
  }

  if (root->left != NULL)
    leftVR = generateFromAST(root->left, (LLVMValue) {NONE, 0}, root->token.type);
  if (root->right != NULL)
    rightVR = generateFromAST(root->right, leftVR, root->token.type);

  switch (root->token.type) {
    case PLUS:
    case MINUS:
    case STAR:
    case SLASH:
    case BITSHIFT_LEFT:
    case BITSHIFT_RIGHT:
      leftVR = generateEnsureRegisterLoaded(leftVR, 0);
      rightVR = generateEnsureRegisterLoaded(rightVR, 0);
      return generateBinaryArithmetic(root->token, leftVR, rightVR);
    case EQ:
    case NEQ:
    case LT:
    case LEQ:
    case GT:
    case GEQ:
      leftVR = generateEnsureRegisterLoaded(leftVR, 0);
      rightVR = generateEnsureRegisterLoaded(rightVR, 0);
      if (parentOperation == IF || parentOperation == WHILE) {
        return generateCompareJump(root->token, leftVR, rightVR, rvalueVR);
      } else {
        return generateComparison(root->token, leftVR, rightVR);
      }
    case NUMBER_LITERAL:
      return generateStoreConstant(root->token.val.num, root->token.valueType.value.number.numType);
    case PRINT:
      generatePrintInt(leftVR);
      return (LLVMValue) {NONE, 0};
    case IDENTIFIER:
      return generateLoadGlobal(root->token.val.string);
    case LEFTVALUE_IDENTIFIER:
      generateStoreGlobal(root->token.val.string, rvalueVR);
      return (LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType, rvalueVR.pointerDepth};
    case ASSIGN:
      return (LLVMValue) {VIRTUAL_REGISTER, rvalueVR.val, rvalueVR.numType, rvalueVR.pointerDepth};
    case LABEL_TOKEN:
      {
        LLVMValue label = (LLVMValue) {LABEL, root->token.val.num};
        generateJump(label);
        generateLabel(label);
        return (LLVMValue) {NONE, 0};
      }
    case BREAK:
      if (BREAK_LABELS == NULL)
        fatal(RC_ERROR, "Break statement not valid here!\n");
      LLVM_VIRTUAL_REGISTER_NUMBER++; // TODO WHY??
      generateJump(BREAK_LABELS->val);
      return (LLVMValue) {NONE, 0};
    case CONTINUE:
      if (CONTINUE_LABELS == NULL)
        fatal(RC_ERROR, "Continue statement not valid here!\n");
      LLVM_VIRTUAL_REGISTER_NUMBER++;
      generateJump(CONTINUE_LABELS->val);
      return (LLVMValue) {NONE, 0};
    case RETURN:
      generateReturn(leftVR, root->token.val.string);
      return (LLVMValue) {NONE, 0};
    case FUNCTION_CALL:
      return generateFunctionCall(root->token.val.string, leftVR);
    case AMPERSAND:
      return generateGetAddress(root->token.val.string);
    case DEREFERENCE:
      return generateDereference(leftVR);
    case UNKNOWN_TOKEN:
      return (LLVMValue) {NONE};
    default:
      fatal(RC_ERROR, "Encountered bad token type while evaluating ASTNode: %s", TOKENTYPE_STRING[root->token.type]);
      return leftVR; 
  }
}

LLVMValue generateIf(ASTNode* root) {
  LLVMValue endLabel;
  LLVMValue falseLabel = getNextLabel();
  if (root->right != NULL)
    endLabel = getNextLabel();

  if (root->left != NULL && root->center != NULL) {
    generateFromAST(root->left, falseLabel, root->token.type);
    generateFromAST(root->center, (LLVMValue) {NONE}, root->token.type);
  } else {
    fatal(RC_ERROR, "ASTNode missing left or middle child\n");
  }

  if (root->right != NULL) {
    generateJump(endLabel);
  } else {
    generateJump(falseLabel);
  }

  generateLabel(falseLabel);

  if (root->right != NULL) {
    generateFromAST(root->right, (LLVMValue) {NONE}, root->token.type);

    generateJump(endLabel);
    generateLabel(endLabel);
  }

  return (LLVMValue) {NONE};
}

LLVMValue generateWhile(ASTNode* root) {
  LLVMValue conditionLabel = getNextLabel();
  LLVMValue endLabel = getNextLabel();

  pushBreakLabel(endLabel);

  // Continue label depends on whether for or while loop
  if (root->right->token.type == AST_GLUE && root->right->right->left->token.type == LABEL_TOKEN) { // This is a for loop
    LLVMValue postambleLabel = (LLVMValue) {LABEL, root->right->right->left->token.val.num};
    pushContinueLabel(postambleLabel);
  } else { // This is a while loop
    pushContinueLabel(conditionLabel);
  }

  generateJump(conditionLabel);
  generateLabel(conditionLabel);

  generateFromAST(root->left, endLabel, root->token.type);
  generateFromAST(root->right, (LLVMValue) {NONE}, root->token.type);

  generateJump(conditionLabel);
  generateLabel(endLabel);

  popContinueLabel();
  popBreakLabel();

  return (LLVMValue) {NONE};
}

void generateDeclareGlobal(char* name, int value, Number num) {
  num.pointerDepth--;

  if (num.pointerDepth == 0) {
    fprintf(LLVM_GLOBALS_OUTPUT, "@%s = global %s %d\n", name, numberToLLVM(num), value);
  } else {
    fprintf(LLVM_GLOBALS_OUTPUT, "@%s = global %s null\n", name, numberToLLVM(num));
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
  
  while (GLOBAL_TOKEN.type != END) {
    // reinit
    LLVM_VIRTUAL_REGISTER_NUMBER = 0;
    LLVM_FREE_REGISTERS = NULL;
    CONTINUE_LABELS = NULL;
    BREAK_LABELS = NULL;

    ASTNode* root = parseFunctionDeclaration();
    generateFromAST(root, (LLVMValue) {NONE}, root->token.type);
  }

  generatePostamble();

  fclose(LLVM_OUTPUT);
  fclose(LLVM_GLOBALS_OUTPUT);

  injectGlobals();
}
