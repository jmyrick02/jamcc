#include "token.h"

#pragma once
typedef enum {
  VIRTUAL_REGISTER = 0,
  LABEL,
  CONSTANT,
  NONE,
} LLVMValueType;

#pragma once
typedef struct LLVMValue {
  LLVMValueType type;
  long val;
  NumberType numType;
  int pointerDepth;
} LLVMValue;

#pragma once
typedef struct LLVMNode {
  LLVMValue val;
  int alignBytes;
  struct LLVMNode* next;
} LLVMNode;
