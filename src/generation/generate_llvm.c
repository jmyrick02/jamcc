#include "../../include/generation/generate_llvm.h"
#include "../../include/parsing/function_declaration.h"
#include "../../include/parsing/optimization.h"

FILE* LLVM_OUTPUT;
int LLVM_VIRTUAL_REGISTER_NUMBER = 0;
LLVMNode* LLVM_FREE_REGISTERS = NULL;
int LLVM_LABEL_INDEX = 0;

LLVMNode* CONTINUE_LABELS = NULL;
LLVMNode* BREAK_LABELS = NULL;

FILE* LLVM_GLOBALS_OUTPUT;

extern Token GLOBAL_TOKEN;

extern char* ARG_FILEPATH;

#define TARGET_DATALAYOUT "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
#define TARGET_TRIPLE "x86_64-pc-linux-gnu"
#define ATTRIBUTES_0 "noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\""
#define CLANG_VERSION "clang version 15.0.7"

#define LLVM_GLOBALS_INJECTION_IDENTIFIER "<JAMCC GLOBALS PLACEHOLDER - If you see this, an issue with jamcc occurred>\n"

void pushFreeRegister(LLVMValue vr) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  *newNode = CONSTRUCTOR_LLVMNODE(vr, LLVM_FREE_REGISTERS);
  
  LLVM_FREE_REGISTERS = newNode;
}

void pushContinueLabel(LLVMValue label) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  *newNode = CONSTRUCTOR_LLVMNODE(label, CONTINUE_LABELS);

  CONTINUE_LABELS = newNode;
}

void pushBreakLabel(LLVMValue label) {
  LLVMNode* newNode = malloc(sizeof(LLVMNode));
  *newNode = CONSTRUCTOR_LLVMNODE(label, BREAK_LABELS);

  BREAK_LABELS = newNode;
}

int typesEqual(Type type1, Type type2) {
  if (type1.type != type2.type)
    return 0;
  
  switch (type1.type) {
    case NUMBER_TYPE:
      return type1.value.number.numType == type2.value.number.numType && type1.value.number.pointerDepth == type2.value.number.pointerDepth;
    case FUNCTION_TYPE:
      return type1.value.function.returnType == type2.value.function.returnType; // TODO check if arguments are the same
    case ARRAY_TYPE:
      return typesEqual(CONSTRUCTOR_TYPE_FROM_NUMBER(type1.value.array.num), CONSTRUCTOR_TYPE_FROM_NUMBER(type2.value.array.num)) && type1.value.array.length == type2.value.array.length;
    default:
      fatal(RC_ERROR, "Cannot compare given types\n");
      return 0;
  }
}

LLVMValue popFreeRegister(Type dataType) {
  LLVMNode* cur = LLVM_FREE_REGISTERS;
  LLVMNode* prev = NULL;
  while (cur != NULL) {
    if (typesEqual(cur->val.dataType, dataType)) 
      break;

    prev = cur;
    cur = cur->next;
  }
  if (cur == NULL)
    fatal(RC_ERROR, "No register of given type\n");
  
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
  return CONSTRUCTOR_LLVMVALUE_LABEL(++LLVM_LABEL_INDEX);
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

char* arrayToLLVM(Array array) {
  char *numTypeStr = numberToLLVM(array.num);

  int length = 1 + 20 + 3 + strlen(numTypeStr) + 1;
  char *result = malloc(sizeof(char) * length);
  
  sprintf(result, "[%d x %s]", array.length, numTypeStr);
  return result;
}

char* typeToLLVM(Type type) {
  switch (type.type) {
    case NUMBER_TYPE:
      return numberToLLVM(type.value.number);
    case ARRAY_TYPE:
      return arrayToLLVM(type.value.array);
    default:
      fatal(RC_ERROR, "Type has no LLVM representation\n");
      return "";
  }
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

char* LLVMValueToLLVM(LLVMValue val) {
  char* result;
  if (val.llvmType == VIRTUAL_REGISTER) {
    int length = 1 + 20 + 1; // 20 max length of long
    result = malloc(sizeof(char) * length);
    if (strcmp(val.name, "") == 0) {
      sprintf(result, "%%%ld", val.val);
    } else {
      sprintf(result, "%%%s", val.name);
    }
  } else if (val.llvmType == LABEL) {
    int length = 1 + 20 + 1; // 20 max length of long
    result = malloc(sizeof(char) * length);
    sprintf(result, "L%ld", val.val);
  } else if (val.llvmType == CONSTANT) {
    int length = 20 + 1; // 20 max length of long
    result = malloc(sizeof(char) * length);
    sprintf(result, "%ld", val.val);
  } else {
    fatal(RC_ERROR, "LLVMValue has no LLVM representation\n");
  }

  return result;
}

int isLocalVar(LLVMValue vr) {
  return vr.llvmType == VIRTUAL_REGISTER && strcmp(vr.name, "") != 0;
}

char *functionArgsTypeRepresentation(Function function) {
  int argc = 0;
  ArgumentNode *cur = function.args;
  while (cur != NULL) {
    argc++;
    cur = cur->next;
  }

  char* result = malloc(sizeof(char) * argc * (4 + 2 + 1 + 2));
  int index = 0;

  cur = function.args;
  while (cur != NULL) {
    if (cur != function.args) {
      result[index++] = ',';
      result[index++] = ' ';
    }

    int length = strlen(NUMBERTYPE_LLVM[cur->numType]) + 2 + strlen(cur->name);
    char* argRepr = malloc(sizeof(char) * length);
    sprintf(argRepr, "%s", NUMBERTYPE_LLVM[cur->numType]);

    int argReprActualLength = strlen(argRepr);
    for (int i = 0; i < argReprActualLength; i++) {
      result[index++] = argRepr[i];
    }
    
    cur = cur->next;
  }
  result[index] = '\0';

  return result;
}

char* functionArgsLLVMRepr(Function function) {
  int argc = 0;
  ArgumentNode *cur = function.args;
  while (cur != NULL) {
    argc++;
    cur = cur->next;
  }

  char* result = malloc(sizeof(char) * argc * (4 + 2 + MAX_IDENTIFIER_LENGTH + 1 + 2));
  int index = 0;

  cur = function.args;
  while (cur != NULL) {
    if (cur != function.args) {
      result[index++] = ',';
      result[index++] = ' ';
    }

    int length = strlen(NUMBERTYPE_LLVM[cur->numType]) + 2 + strlen(cur->name);
    char* argRepr = malloc(sizeof(char) * length);
    sprintf(argRepr, "%s %%%d", NUMBERTYPE_LLVM[cur->numType], getNextVirtualRegisterNumber() - 1);

    int argReprActualLength = strlen(argRepr);
    for (int i = 0; i < argReprActualLength; i++) {
      result[index++] = argRepr[i];
    }
    
    cur = cur->next;
  }
  result[index] = '\0';

  return result;
}

char* argsLLVMNodeLLVMRepr(LLVMNode *args, char* functionName) {
  int argc = 0;
  LLVMNode *cur = args;
  while (cur != NULL) {
    argc++;
    cur = cur->next;
  }

  char* result = malloc(sizeof(char) * argc * 4 * (4 + 2 + MAX_IDENTIFIER_LENGTH + 1 + 2)); // TODO scuffed af
  int index = 0;

  SymbolTableEntry *entry = getGlobal(functionName);
  if (entry == NULL)
    fatal(RC_ERROR, "Function does not exist\n");
  ArgumentNode *curArgNode = entry->type.value.function.args;

  cur = args;
  while (cur != NULL) {
    if (cur != args) {
      result[index++] = ',';
      result[index++] = ' ';
    }

    int length = 4 + 1 + 1 + strlen(cur->val.name) + 1;
    char* curRepr = malloc(length);
    sprintf(curRepr, "%s %s", numberToLLVM(CONSTRUCTOR_NUMBER(curArgNode->numType)), LLVMValueToLLVM(cur->val));

    int curReprActualLength = strlen(curRepr);
    for (int i = 0; i < curReprActualLength; i++) {
      result[index++] = curRepr[i];
    }
    
    curArgNode = curArgNode->next;
    cur = cur->next;
  }
  result[index] = '\0';

  return result;
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
    Type type;
    if (root->token.type == NUMBER_LITERAL) {
      type = root->token.valueType;
    } else if (root->token.type == IDENTIFIER) {
      SymbolTableEntry* entry = getTables(root->token.val.string);
      if (entry == NULL)
        fatal(RC_ERROR, "Undeclared variable %s while generating stack entries\n", root->token.val.string);

      type = entry->type;
    } else {
      return NULL;
    }

    LLVMNode* result = malloc(sizeof(LLVMNode));
    *result = CONSTRUCTOR_LLVMNODE_ALIGNED(CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), type), 4, NULL);

    pushFreeRegister(result->val);

    return result;
  }
}

void generateStackAllocation(LLVMNode* head) {
  LLVMNode* cur = head;
  while (cur != NULL) {
    fprintf(LLVM_OUTPUT, "\t%s = alloca %s, align %d\n", LLVMValueToLLVM(cur->val), typeToLLVM(cur->val.dataType), cur->alignBytes);
    cur = cur->next;
  }
}

void generatePreamble() {
  fprintf(LLVM_OUTPUT, "; ModuleID = '%s'\n", ARG_FILEPATH);
  fprintf(LLVM_OUTPUT, "source_filename = \"%s\"\n", ARG_FILEPATH);
  fprintf(LLVM_OUTPUT, "target datalayout = \"%s\"\n", TARGET_DATALAYOUT);
  fprintf(LLVM_OUTPUT, "target triple = \"%s\"\n", TARGET_TRIPLE);
  fprintf(LLVM_OUTPUT, "\n");
  fprintf(LLVM_OUTPUT, "@print_int_fstring = private unnamed_addr constant [4 x i8] c\"%%d\\0A\\00\", align 1\n");
  fprintf(LLVM_OUTPUT, "define dso_local i32 @printint(i32 %%value) {\n");
  fprintf(LLVM_OUTPUT, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @print_int_fstring , i32 0, i32 0), i32 %%value)\n");
  fprintf(LLVM_OUTPUT, "\tret i32 %%value\n");
  fprintf(LLVM_OUTPUT, "}\n");
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

LLVMValue generateEnsureRegisterLoaded(LLVMValue vr, int loadLevel) {
  if (loadLevel < 0)
    loadLevel = 0;

  // Check if register is already loaded
  if (vr.dataType.value.number.pointerDepth <= loadLevel)
    return vr;

  // Load the register since it's unloaded
  Type newType = vr.dataType;
  newType.value.number.pointerDepth--;
  LLVMValue newVR = CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), newType);
  newVR.lastLoaded = vr.lastLoaded;

  fprintf(LLVM_OUTPUT, "\t; generateEnsureRegisterLoaded\n");
  fprintf(LLVM_OUTPUT, "\t%s = load %s, %s %s\n", LLVMValueToLLVM(newVR), typeToLLVM(newVR.dataType), typeToLLVM(vr.dataType), LLVMValueToLLVM(vr));

  return generateEnsureRegisterLoaded(newVR, loadLevel);
}

LLVMValue generateIntResize(LLVMValue vr, NumberType newWidth) {
  if (vr.dataType.value.number.numType == newWidth)
    return vr;

  if (vr.llvmType == CONSTANT) {
    vr.dataType.value.number.numType = newWidth;
    if (newWidth == NUM_CHAR && vr.val > MAX_CHAR_VALUE)
      vr.val = MAX_CHAR_VALUE;
    else if (newWidth == NUM_SHORT && vr.val > MAX_SHORT_VALUE)
      vr.val = MAX_SHORT_VALUE;
    else if (newWidth == NUM_INT && vr.val > MAX_INT_VALUE)
      vr.val = MAX_INT_VALUE;
    else if (newWidth == NUM_LONG && vr.val > MAX_LONG_VALUE)
      vr.val = MAX_LONG_VALUE;

    return vr;
  }

  vr = generateEnsureRegisterLoaded(vr, 0);

  int outVR = getNextVirtualRegisterNumber();
  if (NUMBERTYPE_SIZE[newWidth] > NUMBERTYPE_SIZE[vr.dataType.value.number.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = zext i%d %s to i%d\n", outVR, NUMBERTYPE_SIZE[vr.dataType.value.number.numType], LLVMValueToLLVM(vr), NUMBERTYPE_SIZE[newWidth]);
  } else if (NUMBERTYPE_SIZE[newWidth] < NUMBERTYPE_SIZE[vr.dataType.value.number.numType]) {
    fprintf(LLVM_OUTPUT, "\t%%%d = trunc i%d %%%ld to i%d\n", outVR, NUMBERTYPE_SIZE[vr.dataType.value.number.numType], vr.val, NUMBERTYPE_SIZE[newWidth]);
  }
  
  return CONSTRUCTOR_LLVMVALUE_VR(outVR, CONSTRUCTOR_NUMBER_TYPE(newWidth));
}

void generateStoreLocal(SymbolTableEntry *entry, LLVMValue rvalue) {
  if (entry == NULL)
    fatal(RC_ERROR, "Storing in nonexistent local variable\n");

  if (rvalue.llvmType == CONSTANT) {
    LLVMValue lvar = entry->latestLLVMValue;

    rvalue = generateEnsureRegisterLoaded(rvalue, lvar.dataType.value.number.pointerDepth - 1);
    rvalue = generateIntResize(rvalue, lvar.dataType.value.number.numType);

    fprintf(LLVM_OUTPUT, "\t; generateStoreLocal\n");
    fprintf(LLVM_OUTPUT, "\tstore %s %s, %s %s\n", typeToLLVM(rvalue.dataType), LLVMValueToLLVM(rvalue), typeToLLVM(lvar.dataType), LLVMValueToLLVM(lvar));
  } else { // rvalue is a register
    entry->latestLLVMValue = rvalue;
  }
}

void generateStoreLocalIntoLLVMValue(LLVMValue var, LLVMValue rvalue) {
  rvalue = generateEnsureRegisterLoaded(rvalue, var.dataType.value.number.pointerDepth - 1);
  rvalue = generateIntResize(rvalue, var.dataType.value.number.numType);

  fprintf(LLVM_OUTPUT, "\t; generateStoreLocalIntoLLVMValue\n");
  fprintf(LLVM_OUTPUT, "\tstore %s %s, %s %s\n", typeToLLVM(rvalue.dataType), LLVMValueToLLVM(rvalue), typeToLLVM(var.dataType), LLVMValueToLLVM(var));
}

LLVMValue generateStoreConstant(long constant, NumberType type) {
  LLVMValue vr = popFreeRegister(CONSTRUCTOR_NUMBER_TYPE(type));
  fprintf(LLVM_OUTPUT, "\t; generateStoreConstant\n");
  fprintf(LLVM_OUTPUT, "\tstore %s %ld, %s* %%%ld\n", NUMBERTYPE_LLVM[type], constant, NUMBERTYPE_LLVM[type], vr.val);

  vr.dataType.value.number.pointerDepth = 1;
  return generateEnsureRegisterLoaded(vr, 0);
}

LLVMValue generateAdd(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber(); 
  fprintf(LLVM_OUTPUT, "\t%%%d = add nsw %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateSub(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = sub nsw %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateMul(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = mul nsw %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateDiv(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = udiv %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateShl(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = shl nsw %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateAshr(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = ashr %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateAnd(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = and %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateOr(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = or %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateXor(LLVMValue leftVR, LLVMValue rightVR) {
  int outVRNum = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t%%%d = xor %s %s, %s\n", outVRNum, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  return CONSTRUCTOR_LLVMVALUE_VR(outVRNum, leftVR.dataType);
}

LLVMValue generateComparison(Token comparison, LLVMValue leftVR, LLVMValue rightVR) {
  // Check for size incompatibilities
  if (NUMBERTYPE_SIZE[leftVR.dataType.value.number.numType] < NUMBERTYPE_SIZE[rightVR.dataType.value.number.numType]) {
    leftVR = generateIntResize(leftVR, rightVR.dataType.value.number.numType); // Extend leftVR to rightVR's num type
  } else if (NUMBERTYPE_SIZE[leftVR.dataType.value.number.numType] > NUMBERTYPE_SIZE[rightVR.dataType.value.number.numType]) {
    rightVR = generateIntResize(rightVR, leftVR.dataType.value.number.numType); // Extend rightVR to leftVR's num type
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

  fprintf(LLVM_OUTPUT, "\t%%%d = icmp %s %s %s, %s\n", boolOutVR, op, typeToLLVM(leftVR.dataType), LLVMValueToLLVM(leftVR), LLVMValueToLLVM(rightVR));

  // Extend bool (i1) to int (i32)
  return generateIntResize(CONSTRUCTOR_LLVMVALUE_VR(boolOutVR, CONSTRUCTOR_NUMBER_TYPE(NUM_BOOL)), NUM_INT);
}

LLVMValue generateBinaryArithmetic(Token token, LLVMValue leftVR, LLVMValue rightVR) {
  // Check for size incompatibilities
  if (NUMBERTYPE_SIZE[leftVR.dataType.value.number.numType] < NUMBERTYPE_SIZE[rightVR.dataType.value.number.numType]) {
    leftVR = generateIntResize(leftVR, rightVR.dataType.value.number.numType); // Extend leftVR to rightVR's num type
  } else if (NUMBERTYPE_SIZE[leftVR.dataType.value.number.numType] > NUMBERTYPE_SIZE[rightVR.dataType.value.number.numType]) {
    rightVR = generateIntResize(rightVR, leftVR.dataType.value.number.numType); // Extend rightVR to leftVR's num type
  }
    
  // Handle constants
  if (leftVR.llvmType == CONSTANT && rightVR.llvmType == CONSTANT) {
    long result = evaluate(token.type, leftVR.val, rightVR.val);
    return CONSTRUCTOR_LLVMVALUE_CONSTANT(result, leftVR.dataType.value.number.numType);
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
    case BITWISE_AND:
      result = generateAnd(leftVR, rightVR);
      break;
    case BITWISE_OR:
      result = generateOr(leftVR, rightVR);
      break;
    case BITWISE_XOR:
      result = generateXor(leftVR, rightVR);
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
  SymbolTableEntry* globalEntry = getTables(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s encountered while loading global\n", string);

  int outVR = getNextVirtualRegisterNumber();
  fprintf(LLVM_OUTPUT, "\t; generateLoadGlobal\n");
  Type newType = globalEntry->type;
  newType.value.number.pointerDepth--;
  fprintf(LLVM_OUTPUT, "\t%%%d = load %s, %s @%s\n", outVR, typeToLLVM(newType), typeToLLVM(globalEntry->type), string);
  return CONSTRUCTOR_LLVMVALUE_VR_LAST_LOADED(outVR, newType, string);
}

void generateStoreGlobal(char* string, LLVMValue rvalueVR) {
  printf("Storing global %s\n", string);
  SymbolTableEntry* globalEntry = getTables(string);
  if (globalEntry == NULL)
    fatal(RC_ERROR, "Undeclared variable %s encountered while storing global\n", string);

  LLVMValue outVR = generateEnsureRegisterLoaded(CONSTRUCTOR_LLVMVALUE_VR(rvalueVR.val, rvalueVR.dataType), globalEntry->type.value.number.pointerDepth - 1);
  if (outVR.dataType.value.number.numType != globalEntry->type.value.number.numType)
    outVR = generateIntResize(outVR, globalEntry->type.value.number.numType);

  fprintf(LLVM_OUTPUT, "\t; generateStoreGlobal\n");
  fprintf(LLVM_OUTPUT, "\tstore %s %%%ld, %s @%s\n", typeToLLVM(outVR.dataType), outVR.val, typeToLLVM(globalEntry->type), string);
}

LLVMValue generateDeclareLocal(char* name, Type type) {
  if (type.type != NUMBER_TYPE && type.type != ARRAY_TYPE)
    fatal(RC_ERROR, "Invalid type for local variable\n");

  LLVMValue outVR = CONSTRUCTOR_LLVMVALUE_VR(-1, type);
  outVR.name = malloc(sizeof(char) * strlen(name));
  strcpy(outVR.name, name);

  LLVMNode* temp = malloc(sizeof(LLVMNode));
  temp->val = outVR;
  temp->alignBytes = 4;
  temp->next = NULL;
  generateStackAllocation(temp);

  outVR.dataType.value.number.pointerDepth++;

  return outVR;
}

void generateStoreDereference(LLVMValue destination, LLVMValue value) {
  // Load local variables once
  if (isLocalVar(value))
    value = generateEnsureRegisterLoaded(value, value.dataType.value.number.pointerDepth - 1);

  destination = generateEnsureRegisterLoaded(destination, value.dataType.value.number.pointerDepth + 1);

  fprintf(LLVM_OUTPUT, "\tgenerateStoreDereference\n");
  if (strcmp(destination.lastLoaded, "") == 0 || destination.dataType.value.number.pointerDepth == value.dataType.value.number.pointerDepth + 1) {
    fprintf(LLVM_OUTPUT, "\tstore %s %s, %s %%%ld\n", typeToLLVM(value.dataType), LLVMValueToLLVM(value), typeToLLVM(destination.dataType), destination.val);
  } else {
    fprintf(LLVM_OUTPUT, "\tstore %s %s, %s* @%s\n", typeToLLVM(value.dataType), LLVMValueToLLVM(value), typeToLLVM(destination.dataType), destination.lastLoaded);
  }
}

void generateLabel(LLVMValue label) {
  if (label.llvmType != LABEL)
    fatal(RC_ERROR, "Expected label for label\n");

  fprintf(LLVM_OUTPUT, "\t%s:\n", LLVMValueToLLVM(label));
}

void generateJump(LLVMValue label) {
  if (label.llvmType != LABEL)
    fatal(RC_ERROR, "Expected label for jump\n");

  fprintf(LLVM_OUTPUT, "\tbr label %%%s\n", LLVMValueToLLVM(label));
}

void generateConditionalJump(LLVMValue conditionVR, LLVMValue trueLabel, LLVMValue falseLabel) {
  if (trueLabel.llvmType != LABEL || falseLabel.llvmType != LABEL)
    fatal(RC_ERROR, "Expected labels in conditional jump\n");
  
  fprintf(LLVM_OUTPUT, "\tbr %s %%%ld, label %%%s, label %%%s\n", typeToLLVM(conditionVR.dataType), conditionVR.val, LLVMValueToLLVM(trueLabel), LLVMValueToLLVM(falseLabel));
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
  SymbolTableEntry* entry = getTables(functionName);
  if (entry == NULL)
    fatal(RC_ERROR, "Tried to return for undeclared function %s\n", functionName);

  if (entry->type.value.function.returnType == VOID) {
    fprintf(LLVM_OUTPUT, "\tret void\n");
  } else {
    returnValue = generateEnsureRegisterLoaded(returnValue, 0);
    fprintf(LLVM_OUTPUT, "\tret %s %s\n", typeToLLVM(returnValue.dataType), LLVMValueToLLVM(returnValue)); 
  }

  LLVM_VIRTUAL_REGISTER_NUMBER++;
}

LLVMValue generateFunctionCall(char* name, LLVMNode *args) {
  LLVMValue result = CONSTRUCTOR_LLVMVALUE_NONE; 

  SymbolTableEntry* entry = getGlobal(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Generating call for undeclared function %s\n", name);
  if (entry->type.type != FUNCTION_TYPE)
    fatal(RC_ERROR, "Calling a non-function identifier %s as a function\n", name);

  LLVMNode *cur = args;
  while (cur != NULL) {
    cur->val = generateEnsureRegisterLoaded(cur->val, 0); 
    cur = cur->next;
  }

  fprintf(LLVM_OUTPUT, "\t");

  if (entry->type.value.function.returnType != VOID) {
    result = CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), CONSTRUCTOR_NUMBER_TYPE(tokenTypeToNumberType(entry->type.value.function.returnType)));

    fprintf(LLVM_OUTPUT, "%%%ld = ", result.val);
  }

  fprintf(LLVM_OUTPUT, "call %s (%s) @%s(%s)\n", tokenTypeToLLVM(entry->type.value.function.returnType), functionArgsTypeRepresentation(entry->type.value.function), name, argsLLVMNodeLLVMRepr(args, name));
  return result;
}

LLVMValue generateGetAddress(char* identifier) {
  SymbolTableEntry* entry = getTables(identifier);
  if (entry == NULL)
    fatal(RC_ERROR, "Getting address of undeclared identifier %s\n", identifier);
  if (entry->type.type != NUMBER_TYPE && entry->type.type != ARRAY_TYPE)
    fatal(RC_ERROR, "Can only get the address of numbers and arrays\n");

  LLVMValue lv = CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), entry->latestLLVMValue.dataType);

  lv.dataType.value.number.pointerDepth--;

  fprintf(LLVM_OUTPUT, "\t%s = getelementptr inbounds %s, %s %s\n", LLVMValueToLLVM(lv), typeToLLVM(lv.dataType), typeToLLVM(entry->latestLLVMValue.dataType), LLVMValueToLLVM(entry->latestLLVMValue));

  lv.dataType.value.number.pointerDepth++;
  return lv;
}

LLVMValue generateDereference(LLVMValue value) {
  Type newType = value.dataType;
  newType.value.number.pointerDepth--;
  LLVMValue out = CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), newType);

  fprintf(LLVM_OUTPUT, "\t; generateDereference\n");
  fprintf(LLVM_OUTPUT, "\t%s = load %s, %s %s\n", LLVMValueToLLVM(out), typeToLLVM(out.dataType), typeToLLVM(value.dataType), LLVMValueToLLVM(value));

  return out;
}

LLVMValue generateArrayAccess(char* name, LLVMValue offset) {
  SymbolTableEntry *entry = getTables(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Accessing non-existent array %s\n", name);
  if (entry->type.type != ARRAY_TYPE)
    fatal(RC_ERROR, "Attempting to access non-array %s as an array\n", name);

  offset = generateIntResize(offset, NUM_LONG);

  LLVMValue lv = CONSTRUCTOR_LLVMVALUE_VR(getNextVirtualRegisterNumber(), CONSTRUCTOR_TYPE_FROM_NUMBER(entry->type.value.array.num));

  fprintf(LLVM_OUTPUT, "\t%s = getelementptr inbounds %s, %s* %s, i64 0, %s %s\n", LLVMValueToLLVM(lv), typeToLLVM(entry->type), typeToLLVM(entry->type), LLVMValueToLLVM(entry->latestLLVMValue), typeToLLVM(offset.dataType), LLVMValueToLLVM(offset));

  lv.dataType.value.number.pointerDepth++;
  return lv;
}

void generateFunctionPreamble(char* name) {
  SymbolTableEntry* entry = getGlobal(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Generating undeclared function preamble\n");
  
  fprintf(LLVM_OUTPUT, "define dso_local %s @%s(%s) #0 {\n", tokenTypeToLLVM(entry->type.value.function.returnType), name, functionArgsLLVMRepr(entry->type.value.function));

  ArgumentNode *cur = entry->type.value.function.args;
  int i = 0;
  while (cur != NULL) {
    LLVMValue arg = CONSTRUCTOR_LLVMVALUE_VR(-1, CONSTRUCTOR_NUMBER_TYPE(cur->numType));
    arg.name = malloc(sizeof(char) * strlen(cur->name));
    strcpy(arg.name, cur->name);

    LLVMNode* temp = malloc(sizeof(LLVMNode));
    temp->val = arg;
    temp->alignBytes = 4;
    temp->next = NULL;
    generateStackAllocation(temp);

    arg.dataType.value.number.pointerDepth++;

    Type newType = arg.dataType;
    newType.value.number.pointerDepth--;
    generateStoreLocalIntoLLVMValue(arg, CONSTRUCTOR_LLVMVALUE_VR(i, newType));

    SymbolTableEntry *entry = getTables(cur->name);
    if (entry == NULL)
      fatal(RC_ERROR, "Lost track of argument with name %s\n", cur->name);
    entry->latestLLVMValue = arg;

    cur = cur->next;
    i++;
  }
}

void generateFunctionPostamble(char* name) {
  SymbolTableEntry* entry = getTables(name);
  if (entry == NULL)
    fatal(RC_ERROR, "Tried to close undeclared function %s\n", name);

  fprintf(LLVM_OUTPUT, "\tret ");

  if (entry->type.value.function.returnType == VOID) {
    fprintf(LLVM_OUTPUT, "void\n");
  } else {
    fprintf(LLVM_OUTPUT, "%s 0\n", tokenTypeToLLVM(entry->type.value.function.returnType));
  }

  fprintf(LLVM_OUTPUT, "}\n\n");
}
LLVMValue generateFromAST(ASTNode* root, LLVMValue valueFromLeftChild, TokenType parentOperation) {
  if (root == NULL)
    return CONSTRUCTOR_LLVMVALUE_NONE;

  LLVMValue leftVR;
  LLVMValue rightVR;

  if (root->token.type == IF) {
    return generateIf(root);
  } else if (root->token.type == WHILE) {
    return generateWhile(root);
  } else if (root->token.type == AST_GLUE) {
    generateFromAST(root->left, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
    generateFromAST(root->center, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
    generateFromAST(root->right, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
    return CONSTRUCTOR_LLVMVALUE_NONE;
  } else if (root->token.type == FUNCTION) {
    generateFunctionPreamble(root->token.val.string);
    generateStackAllocation(getStackEntriesFromBinaryExpression(root));
    generateFromAST(root->left, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
    generateFunctionPostamble(root->token.val.string);
    return CONSTRUCTOR_LLVMVALUE_NONE;
  }

  if (root->left != NULL)
    leftVR = generateFromAST(root->left, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
  if (root->right != NULL)
    rightVR = generateFromAST(root->right, leftVR, root->token.type);

  switch (root->token.type) {
    case PLUS:
    case MINUS:
    case STAR:
    case SLASH:
    case BITSHIFT_LEFT:
    case BITSHIFT_RIGHT:
    case BITWISE_AND:
    case BITWISE_OR:
    case BITWISE_XOR:
      leftVR = generateEnsureRegisterLoaded(leftVR, 0);
      rightVR = generateEnsureRegisterLoaded(rightVR, 0);
      return generateBinaryArithmetic(root->token, leftVR, rightVR);
    case EQ:
    case NEQ:
    case LT:
    case LEQ:
    case GT:
    case GEQ:
      {
        int pointerDepth;
        if (leftVR.dataType.value.number.pointerDepth <= rightVR.dataType.value.number.pointerDepth) {
          pointerDepth = leftVR.dataType.value.number.pointerDepth;
        } else {
          pointerDepth = rightVR.dataType.value.number.pointerDepth;
        }
        if (isLocalVar(leftVR)) {
          pointerDepth--;
        }

        leftVR = generateEnsureRegisterLoaded(leftVR, pointerDepth);
        rightVR = generateEnsureRegisterLoaded(rightVR, pointerDepth);
        if (parentOperation == IF || parentOperation == WHILE) {
          return generateCompareJump(root->token, leftVR, rightVR, valueFromLeftChild);
        } else {
          return generateComparison(root->token, leftVR, rightVR);
        }
      }
    case VAR_DECL:
      {
        SymbolTableEntry *entry = getTables(root->token.val.string);
        if (entry == NULL)
          fatal(RC_ERROR, "Declaring variable '%s' but not in symbol tables\n", root->token.val.string);
        entry->latestLLVMValue = generateDeclareLocal(root->token.val.string, entry->type);
        return CONSTRUCTOR_LLVMVALUE_NONE;
      }
    case NUMBER_LITERAL:
      return generateStoreConstant(root->token.val.num, root->token.valueType.value.number.numType);
    case IDENTIFIER:
      if (root->isRVal || parentOperation == DEREFERENCE) {
        if (getGlobal(root->token.val.string) != NULL) {
          return generateLoadGlobal(root->token.val.string);
        } else {
          SymbolTableEntry* entry = getTables(root->token.val.string);
          if (entry != NULL)
            return entry->latestLLVMValue;
          fatal(RC_ERROR, "No entry for identifier\n");
        }
      }
      return CONSTRUCTOR_LLVMVALUE_NONE;
    case ASSIGN:
      if (root->right != NULL) {
        if (root->right->token.type == IDENTIFIER) {
          if (getGlobal(root->right->token.val.string) != NULL) {
            generateStoreGlobal(root->right->token.val.string, leftVR);
            return leftVR;
          } else {
            SymbolTableEntry *entry = getTables(root->right->token.val.string);
            generateStoreLocalIntoLLVMValue(entry->latestLLVMValue, leftVR);
            return leftVR;
          }
        } else if (root->right->token.type == DEREFERENCE) {
          generateStoreDereference(rightVR, leftVR);
          return leftVR;
        } else if (root->right->token.type == ARRAY_ACCESS) {
          generateStoreLocalIntoLLVMValue(rightVR, leftVR);
          return leftVR;
        }
        fatal(RC_ERROR, "Expected identifier or derefence right child in assignment\n");
      }
      fatal(RC_ERROR, "Expected right child in assignment statement\n");
    case LABEL_TOKEN:
      {
        LLVMValue label = CONSTRUCTOR_LLVMVALUE_LABEL(root->token.val.num);
        generateJump(label);
        generateLabel(label);
        return CONSTRUCTOR_LLVMVALUE_NONE;
      }
    case BREAK:
      if (BREAK_LABELS == NULL)
        fatal(RC_ERROR, "Break statement not valid here!\n");
      LLVM_VIRTUAL_REGISTER_NUMBER++;
      generateJump(BREAK_LABELS->val);
      return CONSTRUCTOR_LLVMVALUE_NONE;
    case CONTINUE:
      if (CONTINUE_LABELS == NULL)
        fatal(RC_ERROR, "Continue statement not valid here!\n");
      LLVM_VIRTUAL_REGISTER_NUMBER++;
      generateJump(CONTINUE_LABELS->val);
      return CONSTRUCTOR_LLVMVALUE_NONE;
    case RETURN:
      generateReturn(leftVR, root->token.val.string);
      return CONSTRUCTOR_LLVMVALUE_NONE;
    case FUNCTION_CALL:
      {
        // Generate llvmvalues from AST args
        LLVMNode *passedLLVMValues = NULL;
        LLVMNode *curNode;

        ASTNode *curAST = root->left;
        while (curAST != NULL) {
          LLVMNode *newNode = malloc(sizeof(LLVMNode));
          generateStackAllocation(getStackEntriesFromBinaryExpression(curAST->right)); // TODO is this necessary?
          *newNode = CONSTRUCTOR_LLVMNODE(generateFromAST(curAST->right, CONSTRUCTOR_LLVMVALUE_NONE, FUNCTION_CALL), NULL);
          if (passedLLVMValues == NULL) {
            passedLLVMValues = newNode;
            curNode = passedLLVMValues;
          } else {
            curNode->next = newNode;
            curNode = curNode->next;
          }

          curAST = curAST->left;
        }

        return generateFunctionCall(root->token.val.string, passedLLVMValues);
      }
    case AMPERSAND:
      return generateGetAddress(root->token.val.string);
    case DEREFERENCE:
      if (root->isRVal) {
        SymbolTableEntry *entry = getTables(root->token.val.string);
        if (entry != NULL && entry->type.type == ARRAY_TYPE)
          return leftVR;
        return generateDereference(generateDereference(leftVR));
      } else {
        return leftVR;
      }
    case ARRAY_ACCESS:
      return generateArrayAccess(root->token.val.string, leftVR);
    case UNKNOWN_TOKEN:
      return CONSTRUCTOR_LLVMVALUE_NONE;
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
    generateFromAST(root->center, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
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
    generateFromAST(root->right, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);

    generateJump(endLabel);
    generateLabel(endLabel);
  }

  return CONSTRUCTOR_LLVMVALUE_NONE; 
}

LLVMValue generateWhile(ASTNode* root) {
  LLVMValue conditionLabel = getNextLabel();
  LLVMValue elseLabel = getNextLabel();
  LLVMValue endLabel = getNextLabel();

  pushBreakLabel(endLabel);

  // Continue label depends on whether for or while loop
  if (root->center->token.type == AST_GLUE && root->center->right->left->token.type == LABEL_TOKEN) { // This is a for loop
    LLVMValue postambleLabel = CONSTRUCTOR_LLVMVALUE_LABEL(root->center->right->left->token.val.num);
    pushContinueLabel(postambleLabel);
  } else { // This is a while loop
    pushContinueLabel(conditionLabel);
  }

  generateJump(conditionLabel);
  generateLabel(conditionLabel);

  generateFromAST(root->left, elseLabel, root->token.type);
  generateFromAST(root->center, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);

  generateJump(conditionLabel);

  generateLabel(elseLabel);
  generateFromAST(root->right, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);
  generateJump(endLabel);

  generateLabel(endLabel);

  popContinueLabel();
  popBreakLabel();

  return CONSTRUCTOR_LLVMVALUE_NONE;
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

    SymbolTable* newTable = malloc(sizeof(SymbolTable));
    *newTable = CONSTRUCTOR_SYMBOL_TABLE;
    pushTable(newTable);
    
    ASTNode* root = parseFunctionDeclaration();
    generateFromAST(root, CONSTRUCTOR_LLVMVALUE_NONE, root->token.type);

    popTable();
  }

  generatePostamble();

  fclose(LLVM_OUTPUT);
  fclose(LLVM_GLOBALS_OUTPUT);

  injectGlobals();
}
