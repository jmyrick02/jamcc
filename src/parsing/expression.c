#include "../../include/parsing/expression.h"
#include "../../include/lib/logging.h"

extern Token GLOBAL_TOKEN;

// Precondition: terminal node token is scanned
ASTNode* parseTerminalNode(FILE* fp) {
	ASTNode* result = malloc(sizeof(ASTNode));

	if (GLOBAL_TOKEN.type != INTEGER_LITERAL) {
		fatal(RC_ERROR, "Expected integer literal but encountered %s", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
	}

	// Create a leaf node
	result->token = GLOBAL_TOKEN;
	result->left = NULL;
	result->right = NULL;

	return result;
}

// Also checks for errors
int checkPrecedence(TokenType tokenType) {
	if (PRECEDENCE[tokenType] == -1) {
		fatal(RC_ERROR, "Expected operator but encountered %s", TOKENTYPE_STRING[tokenType]); 
	}

	return PRECEDENCE[tokenType];
}

ASTNode* prattParse(FILE* fp, int prevPrecedence) {
	ASTNode* left = parseTerminalNode(fp);
	scan(fp);
	
	TokenType tokenType = GLOBAL_TOKEN.type;
	while (tokenType != END && checkPrecedence(tokenType) > prevPrecedence) {
		scan(fp);

		ASTNode* right = prattParse(fp, PRECEDENCE[tokenType]);

		// Join right subtree with current left subtree
		ASTNode* newLeft = malloc(sizeof(ASTNode));
		Token newToken;
		newToken.type = tokenType;
		newLeft->token = newToken;
		newLeft->left = left;
		newLeft->right = right;
		left = newLeft;

		tokenType = GLOBAL_TOKEN.type;
	}

	return left;
}

ASTNode* parseBinaryExpression(FILE* fp) {
	return prattParse(fp, -1);
}
