#include <stdio.h>

#include "token.h"

typedef struct ASTNode {
	Token token;
	struct ASTNode* left;
	struct ASTNode* right;
} ASTNode;

Token scan(FILE* fp);

