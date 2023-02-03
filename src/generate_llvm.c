#include "../include/generate_llvm.h"

FILE* LLVM_OUTPUT;
int LLVM_VIRTUAL_REGISTER_NUMBER = 0;
int LLVM_FREE_REGISTER_COUNT = 0;
LLVMNode* LLVM_LOADED_REGISTERS = NULL;

extern char* ARG_FILEPATH;

// Change these strings to match your target
const char* TARGET_DATALAYOUT = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128";
const char* TARGET_TRIPLE = "x86_64-redhat-linux-gnu";
const char* ATTRIBUTES_0 = "noinline nounwind optnone uwtable \"frame-pointer\"=\"all\" \"min-legal-vector-width\"=\"0\" \"no-trapping-math\"=\"true\" \"stack-protector-buffer-size\"=\"8\" \"target-cpu\"=\"x86-64\" \"target-features\"=\"+cx8,+fxsr,+mmx,+sse,+sse2,+x87\" \"tune-cpu\"=\"generic\"";
const char* CLANG_VERSION = "clang version 15.0.7 (Fedora 15.0.7-1.fc37)";

void appendToLoadedRegisters(LLVMValue vr) {
	LLVMNode* newNode = malloc(sizeof(LLVMNode));
	newNode->vr = vr;
	newNode->next = NULL;
	if (LLVM_LOADED_REGISTERS == NULL) {
		LLVM_LOADED_REGISTERS = newNode;
	} else {
		LLVMNode* cur = LLVM_LOADED_REGISTERS;
		while (cur->next != NULL) {
			cur = cur->next;
		}
		cur->next = newNode;
	}
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
	LLVMValue newVR;
	newVR.type = VIRTUAL_REGISTER;
	newVR.val = getNextVirtualRegisterNumber();

	fprintf(LLVM_OUTPUT, "\t%%%d = load i32, i32* %%%d\n", newVR.val, vr.val);

	appendToLoadedRegisters(newVR);
	return newVR;
}

LLVMValue generateStoreConstant(int constant) {
	fprintf(LLVM_OUTPUT, "\tstore i32 %d, i32* %%%d\n", constant, LLVM_FREE_REGISTER_COUNT);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = LLVM_FREE_REGISTER_COUNT;

	LLVM_FREE_REGISTER_COUNT -= 1;

	return result;
}

LLVMValue generateAdd(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber(); 
	fprintf(LLVM_OUTPUT, "\t%%%d = add nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
}

LLVMValue generateSub(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber();
	fprintf(LLVM_OUTPUT, "\t%%%d = sub nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
}

LLVMValue generateMul(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber();
	fprintf(LLVM_OUTPUT, "\t%%%d = mul nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
}

LLVMValue generateDiv(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber();
	fprintf(LLVM_OUTPUT, "\t%%%d = udiv i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
}

LLVMValue generateShl(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber();
	fprintf(LLVM_OUTPUT, "\t%%%d = shl nsw i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
}

LLVMValue generateAshr(LLVMValue leftVR, LLVMValue rightVR) {
	int outVRNum = getNextVirtualRegisterNumber();
	fprintf(LLVM_OUTPUT, "\t%%%d = ashr i32 %%%d, %%%d\n", outVRNum, leftVR.val, rightVR.val);

	LLVMValue result;
	result.type = VIRTUAL_REGISTER;
	result.val = outVRNum;
	return result;
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

	appendToLoadedRegisters(result);
	return result;
}

LLVMValue generateFromAST(ASTNode* root) {
	LLVMValue leftVR;
	LLVMValue rightVR;

	if (root->left != NULL) {
		leftVR = generateFromAST(root->left);
	}
	if (root->right != NULL) {
		rightVR = generateFromAST(root->right);
	}

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
			return generateStoreConstant(root->token.val);
		default:
			fatal(RC_ERROR, "Encountered bad operand while evaluating expression");
			return leftVR; 
	}
}

void generatePrintInt(LLVMValue vr) {
	fprintf(LLVM_OUTPUT, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @print_int_fstring , i32 0, i32 0), i32 %%%d)\n", vr.val);
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
	}	else { // Root is an integer literal
		LLVMNode* result = malloc(sizeof(LLVMNode));

		LLVMValue vr;
		vr.type = VIRTUAL_REGISTER;
		vr.val = getNextVirtualRegisterNumber();

		result->vr = vr;
		result->alignBytes = 4;
		result->next = NULL;

		LLVM_FREE_REGISTER_COUNT++;

		return result;
	}
}

void generateLLVM(ASTNode* root) {
	generatePreamble();	
	generateStackAllocation(getStackEntriesFromBinaryExpression(root));
	LLVMValue resultVR = generateFromAST(root);
	generatePrintInt(resultVR);
	generatePostamble();
}
