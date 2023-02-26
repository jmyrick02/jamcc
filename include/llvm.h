typedef enum {
	VIRTUAL_REGISTER = 0,
  NONE,
} LLVMValueType;

typedef struct LLVMValue {
	LLVMValueType type;
	int val;
} LLVMValue;

typedef struct LLVMNode {
	LLVMValue vr;
	int alignBytes;
	struct LLVMNode* next;
} LLVMNode;

typedef struct IntNode {
	int val;
	struct IntNode* next;
} IntNode;
