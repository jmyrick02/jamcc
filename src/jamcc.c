#include <stdio.h>

#include "../include/parsing/expression.h"
#include "../include/lib/logging.h"

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
			fatal(RC_ERROR, "Encountered bad operand while evaluating expression");
			return -1;
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fatal(RC_ERROR, "You didn't pass in an input file path");
	}
	char* filepath = argv[1];

	FILE* fp = fopen(filepath, "r");
	if (fp == NULL) {
		fatal(RC_ERROR, "Failed to open file at %s", filepath);
	}

	scan(fp);
	ASTNode* parsedBinaryExpression = parseBinaryExpression(fp);
	printf("%d\n", evaluate(parsedBinaryExpression));
}
