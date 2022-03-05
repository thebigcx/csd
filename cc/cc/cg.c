#include "cc.h"

int cgilit(struct ast *ast)
{
    fprintf(g_out, "\tmov $%d, %%rax\n", ast->iv);
    return 0;
}

int cgadd(int r1, int r2)
{
    fprintf(g_out, "\tadd %%rax, %%rbx\n");
    return 0;
}

int cgbinop(struct ast *ast)
{
    int r1 = cgilit(ast->left);
    int r2 = cgilit(ast->right);
    cgadd(r1, r2);

    return r2;
}

int cg(struct ast *ast)
{
    cgbinop(ast);
}