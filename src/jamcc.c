#include <stdio.h>

#include "../include/parsing/expression.h"

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
		case INTEGER_LITERAL:
			return root->token.val;
		default:
			printf("ERROR: encountered bad operand\n");
			return -1;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("ERROR: you didn't pass in an input file path!\n");
		return 0;
	}
	char* filepath = argv[1];

	FILE* fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("ERROR: failed to open file at %s\n", filepath);
	}

	scan(fp);
	ASTNode* parsedBinaryExpression = parseBinaryExpression(fp);
	printf("%d\n", evaluate(parsedBinaryExpression));
}
