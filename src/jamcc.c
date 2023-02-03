#include <stdio.h>


#include "../include/parsing/expression.h"
#include "../include/generate_llvm.h"

char* ARG_FILEPATH;

extern FILE* GLOBAL_FILE_POINTER;
extern FILE* LLVM_OUTPUT;

int evaluate(ASTNode* root) {
	int leftVal, rightVal;

	if (root->left != NULL) {
		leftVal = evaluate(root->left);
	}
	if (root->right != NULL) {
		rightVal = evaluate(root->right);
	}

	switch (root->token.type) {
		case PLUS:
			return leftVal + rightVal;
		case MINUS:
			return leftVal - rightVal;
		case STAR:
			return leftVal * rightVal;
		case SLASH:
			return leftVal / rightVal;
		case BITSHIFT_LEFT:
			return leftVal << rightVal;
		case BITSHIFT_RIGHT:
			return leftVal >> rightVal;
		case INTEGER_LITERAL:
			return root->token.val;
		default:
			fatal(RC_ERROR, "Encountered bad operand while evaluating expression");
			return -1;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fatal(RC_ERROR, "You didn't pass in an input file path");
	}
	ARG_FILEPATH = argv[1];

	GLOBAL_FILE_POINTER = fopen(ARG_FILEPATH, "r");
	if (GLOBAL_FILE_POINTER == NULL) {
		fatal(RC_ERROR, "Failed to open file at %s", ARG_FILEPATH);
	}

	LLVM_OUTPUT = fopen("out.ll", "w");
	if (LLVM_OUTPUT == NULL) {
		fatal(RC_ERROR, "Failed to create file out.ll");
	}

	scan();
	ASTNode* parsedBinaryExpression = parseBinaryExpression();
	
	generateLLVM(parsedBinaryExpression);

	fclose(GLOBAL_FILE_POINTER);
	fclose(LLVM_OUTPUT);
}
