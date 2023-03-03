#include "token.h"

#pragma once
typedef enum {
  VIRTUAL_REGISTER = 0,
  NONE,
} LLVMValueType;

#pragma once
typedef struct LLVMValue {
  LLVMValueType type;
  int val;
  NumberType numType;
} LLVMValue;

#pragma once
typedef struct LLVMNode {
  LLVMValue vr;
  NumberType numType;
  int alignBytes;
  struct LLVMNode* next;
} LLVMNode;
