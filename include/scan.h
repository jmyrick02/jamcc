#include <stdio.h>

#include "token.h"

#pragma once
typedef struct ASTNode {
  Token token;
  struct ASTNode* left;
  struct ASTNode* right;
} ASTNode;

void scan();

