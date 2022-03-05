#include "cc.h"

struct ast *stmt()
{
    switch (scan().type)
    {
        default: return expr();
    }
}