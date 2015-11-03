#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h>


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
            mpc_ast_print(r.output);
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
