#include "../../include/parsing/expression.h"
#include "../../include/lib/logging.h"

extern FILE* GLOBAL_FILE_POINTER;
extern Token GLOBAL_TOKEN;

// Precondition: terminal node token is scanned
ASTNode* parseTerminalNode() {
	ASTNode* result = malloc(sizeof(ASTNode));

	if (GLOBAL_TOKEN.type == END) {
		fatal(RC_ERROR, "Expected semicolon but encountered end of file");
	} else if (GLOBAL_TOKEN.type != INTEGER_LITERAL) {
		fatal(RC_ERROR, "Expected terminal token but encountered %s", TOKENTYPE_STRING[GLOBAL_TOKEN.type]);
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

ASTNode* prattParse(int prevPrecedence) {
	ASTNode* left = parseTerminalNode();
	scan();
	
	TokenType tokenType = GLOBAL_TOKEN.type;
	while (tokenType != SEMICOLON && tokenType != END && checkPrecedence(tokenType) > prevPrecedence) {
		scan();

		ASTNode* right = prattParse(PRECEDENCE[tokenType]);

		// Join right subtree with current left subtree
		ASTNode* newLeft = malloc(sizeof(ASTNode));
		newLeft->token = (Token) {tokenType, 0};
		newLeft->left = left;
		newLeft->right = right;
		left = newLeft;

		tokenType = GLOBAL_TOKEN.type;
	}
	if (tokenType == END) {
		fatal(RC_ERROR, "Expected a semicolon but found end of file");
	}

	return left;
}

ASTNode* parseBinaryExpression() {
	return prattParse(-1);
}
