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
  char *lastLoaded;
} LLVMValue;

#pragma once
typedef struct LLVMNode {
  LLVMValue val;
  int alignBytes;
  struct LLVMNode* next;
} LLVMNode;

// Constructors

#define CONSTRUCTOR_LLVMVALUE_NONE (LLVMValue) {NONE, -1, NUM_INT, -2, ""}

#define CONSTRUCTOR_LLVMVALUE_LABEL(index) (LLVMValue) {LABEL, index, NUM_INT, -1, ""}

#define CONSTRUCTOR_LLVMVALUE_VR(registerNumber, numType, pointerDepth) (LLVMValue) {VIRTUAL_REGISTER, registerNumber, numType, pointerDepth, ""}

#define CONSTRUCTOR_LLVMVALUE_VR_LAST_LOADED(registerNumber, numType, pointerDepth, last_loaded) (LLVMValue) {VIRTUAL_REGISTER, registerNumber, numType, pointerDepth, last_loaded}

#define CONSTRUCTOR_LLVMVALUE_CONSTANT(v, numType) (LLVMValue) {CONSTANT, v, numType, 0, ""}

#define CONSTRUCTOR_LLVMNODE(val, next) (LLVMNode) {val, -1, next}

#define CONSTRUCTOR_LLVMNODE_ALIGNED(val, alignment, next) (LLVMNode) {val, alignment, next}
