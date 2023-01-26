#include "../../include/parsing/expression.h"

ASTNode* parseTerminalNode(FILE* fp) {
	Token token = scan(fp);
	ASTNode* result = malloc(sizeof(ASTNode));

	if (token.type != INTEGER_LITERAL) {
		printf("ERROR: Expected integer literal but encountered %s", TOKENTYPE_STRING[token.type]);
		return result;
	}

	// Create a leaf node
	result->token = token;
	result->left = NULL;
	result->right = NULL;

	return result;
}

ASTNode* parseBinaryExpression(FILE* fp) {
	ASTNode* left = parseTerminalNode(fp);

	Token operatorToken = scan(fp);
	if (operatorToken.type == END) {
		return left;
	}

	// A well-formed expression will have an operator token in the operatorToken if not END
	// Now parse the right (we have the left and middle)
	ASTNode* right = parseBinaryExpression(fp);

	ASTNode* result = malloc(sizeof(ASTNode));
	result->token = operatorToken;
	result->left = left;
	result->right = right;

	return result;
}

