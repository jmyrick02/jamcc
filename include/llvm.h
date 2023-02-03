typedef enum {
	VIRTUAL_REGISTER = 0,
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
