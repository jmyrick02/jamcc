#pragma once
typedef enum {
  VIRTUAL_REGISTER = 0,
  NONE,
} LLVMValueType;

#pragma once
typedef struct LLVMValue {
  LLVMValueType type;
  int val;
} LLVMValue;

#pragma once
typedef struct LLVMNode {
  LLVMValue vr;
  int alignBytes;
  struct LLVMNode* next;
} LLVMNode;

#pragma once
typedef struct IntNode {
  int val;
  struct IntNode* next;
} IntNode;
