#include "../../include/parsing/optimization.h"

extern int ARG_OPTIMIZATION;

long evaluate(TokenType operation, long leftVal, long rightVal) {
  switch (operation) {
    case PLUS:
      return leftVal + rightVal;
    case MINUS:
      return leftVal - rightVal;
    case STAR:
      return leftVal * rightVal;
    case SLASH:
      return leftVal / rightVal;
    case BITSHIFT_LEFT:
      return leftVal << rightVal;
    case BITSHIFT_RIGHT:
      return leftVal >> rightVal;
    case EQ:
      return leftVal == rightVal;
    case NEQ:
      return leftVal != rightVal;
    case LT:
      return leftVal < rightVal;
    case LEQ:
      return leftVal <= rightVal;
    case GT:
      return leftVal > rightVal;
    case GEQ:
      return leftVal >= rightVal;
    default:
      fatal(RC_ERROR, "Unknown operator while evaluating constant expression\n");
      return -1;
  }
}

ASTNode* optimizeFoldConstantPair(ASTNode* root, int* changed) {
  if (root->left == NULL || root->right == NULL || root->left->token.type != NUMBER_LITERAL || root->right->token.type != NUMBER_LITERAL)
    return root;

  long leftVal = root->left->token.val.num;
  long rightVal = root->right->token.val.num;
  // TODO TEMP because if statements require conditionals
  if (root->token.type == EQ || root->token.type == NEQ || root->token.type == LT || root->token.type == LEQ || root->token.type == GT || root->token.type == GEQ) {
    return root;
  }
  long foldedVal = evaluate(root->token.type, leftVal, rightVal);

  *changed = 1;

  NumberType leftType = root->left->token.valueType.value.number.numType;
  NumberType rightType = root->right->token.valueType.value.number.numType;
  NumberType foldedType;
  if (NUMBERTYPE_SIZE[leftType] >= NUMBERTYPE_SIZE[rightType]) {
    foldedType = leftType;
  } else {
    foldedType = rightType;
  }
  
  ASTNode* result = malloc(sizeof(ASTNode));
  result->token = (Token) {NUMBER_LITERAL, (TokenVal) {foldedVal}, (Type) {NUMBER_TYPE, (TypeValue) {(Number) {foldedType, -1, 0}}}};
  result->left = NULL;
  result->center = NULL;
  result->right = NULL;

  return result;
}

ASTNode* optimizeIdentityRules(ASTNode* root, int* changed) {
  if (root->left == NULL || root->right == NULL)
    return root;

  ASTNode* zero;
  ASTNode* nonzero;
  if (root->left->token.type == NUMBER_LITERAL && root->left->token.val.num == 0) {
    zero = root->left;
    nonzero = root->right;
  } else if (root->right->token.type == NUMBER_LITERAL && root->right->token.val.num == 0) {
    zero = root->right;
    nonzero = root->left;
  } else { // There are no zeros, check for one as the multiplicative identity
    ASTNode* one;
    ASTNode* nonone;
    if (root->left->token.type == NUMBER_LITERAL && root->left->token.val.num == 1) {
      one = root->left;
      nonone = root->right;
    } else if (root->right->token.type == NUMBER_LITERAL && root->right->token.val.num == 1) {
      one = root->right;
      nonone = root->right;
    } else {
      return root;
    }

    switch (root->token.type) {
      case STAR:
        *changed = 1;
        return nonone;
      case SLASH:
        if (one == root->right) {
          *changed = 1;
          return nonone;
        }
      default:
        return root;
    }
  }

  switch (root->token.type) {
    case PLUS: // x + 0 = x
      *changed = 1;
      return nonzero;
    case MINUS: // x - 0 = x and 0 - x = -x
      *changed = 1;
      if (nonzero == root->left) {
        return nonzero;
      } else {
        nonzero->token.val.num *= -1;
        return nonzero;
      }
    case STAR: // 0 * x = 0
      *changed = 1;
      return zero;
    case SLASH: // 0 / x = 0 and x / 0 is fatal
      if (zero == root->right)
        fatal(RC_ERROR, "Division by 0 error\n");
      *changed = 1;
      return zero;
    case BITSHIFT_LEFT: // x << 0 = x and 0 << x = 0
    case BITSHIFT_RIGHT: // x >> 0 = x and 0 >> x = 0
      *changed = 1;
      if (zero == root->right) {
        return nonzero;
      } else {
        return zero;
      }
    default:
      return root;
  }
}

ASTNode* optimizeAssociativity(ASTNode* root, int* changed) {
  if (root->left == NULL || root->right == NULL)
    return root;

  ASTNode* target;
  ASTNode* nontarget;
  if (root->token.type == root->left->token.type) {
    target = root->left;
    nontarget = root->right;
  } else if (root->token.type == root->right->token.type) {
    target = root->right;
    nontarget = root->left;
  } else {
    return root;
  }

  switch (root->token.type) {
    case PLUS:
    case STAR:
      *changed = 1;
      {
        root->left = nontarget;
        root->right = target->left;
        ASTNode* optimizedLeft = optimize(root);

        ASTNode* result = malloc(sizeof(ASTNode));
        result->token = root->token;
        result->left = optimizedLeft;
        result->center = NULL;
        result->right = target->right;
        return result;
      }
    case MINUS:
    case SLASH:
      *changed = 1;
      {
        TokenType temp = root->token.type;
        if (temp == MINUS) {
          root->token.type = PLUS;
        } else {
          root->token.type = STAR;
        }
        root->left = nontarget;
        root->center = NULL;
        root->right = target->right;

        ASTNode* optimizedRight = optimize(root);

        ASTNode* result = malloc(sizeof(ASTNode));
        result->token = root->token;
        result->left = target->left;
        result->center = NULL;
        result->right = optimizedRight;

        return result;
      }
    default:
      return root;
  }
}

ASTNode* optimize(ASTNode* root) {
  if (ARG_OPTIMIZATION == 0 || root == NULL)
    return root;

  root->left = optimize(root->left);
  root->center = optimize(root->center);
  root->right = optimize(root->right);

  int changed;
  do {
    changed = 0;

    root = optimizeFoldConstantPair(root, &changed);
    root = optimizeIdentityRules(root, &changed);
    root = optimizeAssociativity(root, &changed);
  } while (changed && ARG_OPTIMIZATION == 1);

  return root;
}
