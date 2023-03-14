#include <stdio.h>

#include "token.h"
#include "generation/symboltable.h"

#pragma once
typedef struct ASTNode {
  Token token;
  struct ASTNode* left;
  struct ASTNode* center;
  struct ASTNode* right;
} ASTNode;

void scan();

