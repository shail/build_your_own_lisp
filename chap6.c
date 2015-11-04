#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h>

int num_leaves(mpc_ast_t* t)
{
    if (t->children_num < 1) {
        return 1;
    } else {
        int total = 0;

        for (int i = 0; i < t->children_num; i++) {
            total += num_leaves(t->children[i]);
        }
        return total;
    }
}

int num_branches(mpc_ast_t* t)
{
    if (t->children_num < 1) {
        return 0;
    } else {
        int total = t->children_num;

        for (int i = 0; i < t->children_num; i++) {
            total += num_branches(t->children[i]);
        }
        return total;
    }
}

int max_children(mpc_ast_t* t)
{

}

long eval_op(long x, char* op, long y)
{
    /* Use operator string to see which operation to perform */
    if (strcmp(op, "+") == 0) { return x + y; }
    if (strcmp(op, "-") == 0) { return x - y; }
    if (strcmp(op, "*") == 0) { return x * y; }
    if (strcmp(op, "/") == 0) { return x / y; }
    return 0;
}

long eval(mpc_ast_t* t)
{
    /* If tagged as number return it directly */
    if (strstr(t->tag, "number")) {
        return atoi(t->contents);
    }

    /* The operator is always second child. */
    char *op = t->children[1]->contents;

    /* We store the third child in `x` */
    long x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

int main(int argc, char** argv)
{
    /* Create some parsers */
    mpc_parser_t* Number = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define the parsers */
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                                       \
        number   : /-?[0-9]+(\\.[0-9]+)?/;                                                  \
        operator : '+' | '-' | '*' | '/' | '%' | /add/ | /sub/ | /mul/ | /div/ ;            \
        expr     : <number> | '(' <operator> <expr>+ ')' ;                                  \
        lispy    : /^/ <operator> <expr>+ /$/ ;                                             \
    ",
    Number, Operator, Expr, Lispy);

    puts("Sammy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char *input = readline("sammy> ");

        add_history(input);

        /* Attempt to parse user input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            long result = eval(r.output);
            printf("%li\n", result);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise print an error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    /* Undefine and delete our parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;
}
