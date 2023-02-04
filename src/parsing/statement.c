#include "../../include/parsing/statement.h"

extern Token GLOBAL_TOKEN;

void matchToken(TokenType type) {
	if (GLOBAL_TOKEN.type != type) {
		fatal(RC_ERROR, "Expected token type %s\n", TOKENTYPE_STRING[type]);
	}
	scan();
}

ASTNode* parseStatement() {
	if (GLOBAL_TOKEN.type == END) {
		ASTNode* endNode = malloc(sizeof(ASTNode));
		Token endToken;
		endToken.type = END;
		endNode->token = endToken;
		return endNode;
	}
	matchToken(PRINT);
	ASTNode* parsedBinaryExpression = parseBinaryExpression();
	matchToken(SEMICOLON);
	return parsedBinaryExpression;
}
