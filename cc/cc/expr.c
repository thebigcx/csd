#include "cc.h"

#include <stdlib.h>

struct ast *expr()
{
    struct ast *ast = NEW(struct ast);

    // TEMP
    ast->left = NEW(struct ast);
    ast->mid = NEW(struct ast);
    ast->right = NEW(struct ast);

    ast->left->iv = 10;
    ast->mid->op = OP_ADD;
    ast->right->iv = 11;

    return ast;
}