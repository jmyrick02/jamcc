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
  LLVMValueType llvmType;
  long val;
  Type dataType;
  char *lastLoaded;
  char *name;
} LLVMValue;

#pragma once
typedef struct LLVMNode {
  LLVMValue val;
  int alignBytes;
  struct LLVMNode* next;
} LLVMNode;

// Constructors

#define CONSTRUCTOR_LLVMVALUE_NONE (LLVMValue) {NONE, -1}

#define CONSTRUCTOR_LLVMVALUE_LABEL(index) (LLVMValue) {LABEL, index}

#define CONSTRUCTOR_LLVMVALUE_VR(registerNumber, type) (LLVMValue) {VIRTUAL_REGISTER, registerNumber, type, "", ""}

#define CONSTRUCTOR_LLVMVALUE_VR_LAST_LOADED(registerNumber, type, last_loaded) (LLVMValue) {VIRTUAL_REGISTER, registerNumber, type, last_loaded, ""}

#define CONSTRUCTOR_LLVMVALUE_CONSTANT(v, numType) (LLVMValue) {CONSTANT, v, CONSTRUCTOR_NUMBER_TYPE(numType), "", ""}

#define CONSTRUCTOR_LLVMNODE(val, next) (LLVMNode) {val, -1, next}

#define CONSTRUCTOR_LLVMNODE_ALIGNED(val, alignment, next) (LLVMNode) {val, alignment, next}
