#include "../scan.h"
#include "../parsing/expression.h"
#include "../lib/logging.h"
#include "../generation/symboltable.h"
#include  "../generation/generate_llvm.h"

Token matchToken(TokenType type);

Number matchNumType();

ASTNode* parseBlock();
