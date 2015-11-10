#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <editline/readline.h>

/* Create Enumeration of Possible lval Types */
typedef enum { LVAL_NUM, LVAL_DUB, LVAL_ERR } lval_type_t;

/* Create Enumeration of Possible Error Types */
typedef enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM } lval_type_e;

typedef struct {
    lval_type_t type;
    union {
        long num;
        double doub;
        lval_type_e err;
    } val;
} lval;

/* Create a new number type lval */
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.val.num = x;
    return v;
}

lval lval_doub(double x) {
    lval v;
    v.type = LVAL_NUM;
    v.val.doub = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.val.err = x;
    return v;
}

void lval_print(lval v)
{
    switch (v.type) {
        /* In the case the type is a number print it */
        case LVAL_NUM: printf("%li", v.val.num); break;

        case LVAL_DUB: printf("%f", v.val.doub); break;

        /* In the case the type is an error */
        case LVAL_ERR:
        /* Check what type of error it is and then print it */
            if (v.val.err == LERR_DIV_ZERO) {
                printf("Error: Division By Zero!");
            }
            if (v.val.err == LERR_BAD_OP) {
                printf("Error: Invalid Operator!");
            }
            if (v.val.err == LERR_BAD_NUM) {
                printf("Error: Invalid Number!");
            }
        break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

lval eval_num(lval x, char* op, lval y)
{
    long first_number = x.val.num;
    long second_number = y.val.num;
    /* Use operator string to see which operation to perform */
    if (strcmp(op, "+") == 0) { return lval_num(first_number + second_number); }
    if (strcmp(op, "-") == 0) { return lval_num(first_number - second_number); }
    if (strcmp(op, "*") == 0) { return lval_num(first_number * second_number); }
    if (strcmp(op, "/") == 0) {
        return second_number == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(first_number / second_number);
        return lval_num(first_number / second_number);
    }
    if (strcmp(op, "%") == 0) { return lval_num(first_number % second_number); }
    if (strcmp(op, "^") == 0) { return lval_num(pow(first_number, second_number)); }
    if (strcmp(op, "min") == 0) { return (first_number < second_number) ? lval_num(first_number) : lval_num(second_number); }
    if (strcmp(op, "max") == 0) { return (first_number > second_number) ? lval_num(first_number) : lval_num(second_number); }
    return lval_err(LERR_BAD_OP);
}

lval eval_dub(lval x, char* op, lval y)
{
    double first_number = x.val.doub;
    double second_number = y.val.doub;
    /* Use operator string to see which operation to perform */
    if (strcmp(op, "+") == 0) { return lval_num(first_number + second_number); }
    if (strcmp(op, "-") == 0) { return lval_num(first_number - second_number); }
    if (strcmp(op, "*") == 0) { return lval_num(first_number * second_number); }
    if (strcmp(op, "/") == 0) {
        return second_number == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(first_number / second_number);
        return lval_num(first_number / second_number);
    }
    if (strcmp(op, "%") == 0) { return lval_num(fmod(first_number, second_number)); }
    if (strcmp(op, "^") == 0) { return lval_num(pow(first_number, second_number)); }
    if (strcmp(op, "min") == 0) { return (first_number < second_number) ? lval_num(first_number) : lval_num(second_number); }
    if (strcmp(op, "max") == 0) { return (first_number > second_number) ? lval_num(first_number) : lval_num(second_number); }
    return lval_err(LERR_BAD_OP);
}

lval eval_op(lval x, char* op, lval y)
{
    /* If either value is an error return it */
    if (x.type == LVAL_ERR) { return x; }
    if (y.type == LVAL_ERR) { return y; }

    if ((x.type == LVAL_DUB) && (y.type == LVAL_NUM)) {
        y = lval_doub(y.val.num);
    }

    if ((x.type == LVAL_NUM) && (y.type == LVAL_DUB)) {
        x = lval_doub(x.val.num);
    }

    if ((x.type == LVAL_NUM) && (y.type == LVAL_NUM)) {
        return eval_num(x, op, y);
    } else if ((x.type == LVAL_DUB) && (y.type == LVAL_NUM)) {
        return eval_dub(x, op, y);
    }
    return lval_err(LERR_BAD_OP);
}

lval eval_negative(lval x, char* op) {
    if (x.type == LVAL_NUM) {
        if (strcmp(op, "-") == 0) { return lval_num(x.val.num * -1); }
    } else {
        if (strcmp(op, "-") == 0) { return lval_doub(x.val.doub * -1); }
    };
    return lval_num(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t)
{
    /* If tagged as number return it directly */
    if (strstr(t->tag, "number")) {
        /* Check if there is some error in conversion */
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
    }

    if (strstr(t->tag, "double")) {
        /* Check if there is some error in conversion */
        errno = 0;
        double x = strtod(t->contents, NULL);
        return errno != ERANGE ? lval_doub(x) : lval_err(LERR_BAD_NUM);
    }

    /* The operator is always second child. */
    char *op = t->children[1]->contents;

    /* We store the third child in `x` */
    lval x = eval(t->children[2]);

    /* Iterate the remaining children and combining. */
    int i = 3;
    if (t->children_num == i+1) {
        x = eval_negative(x, op);
    }
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
    mpc_parser_t* Double = mpc_new("double");
    mpc_parser_t* Symbol = mpc_new("symbol");
    mpc_parser_t* Sexpr = mpc_new("sexpr");
    mpc_parser_t* Expr = mpc_new("expr");
    mpc_parser_t* Lispy = mpc_new("lispy");

    /* Define the parsers */
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                                                                      \
        number   : /-?[0-9]+/;                                                                             \
        double   : /-?[0-9]+(\\.[0-9]+)?/;                                                                 \
        symbol   : '+' | '-' | '*' | '/' | '%' | '^' | /min/ | /max/ | /add/ | /sub/ | /mul/ | /div/ ;     \
        sexpr    : '(' <expr>* ')' ;                                                                       \
        expr     : <number> | <double> | <symbol> | <sexpr> ;                                              \
        lispy    : /^/ <expr>+ /$/ ;                                                                       \
    ",
    Number, Double, Symbol, Sexpr, Expr, Lispy);

    puts("Sammy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        char *input = readline("sammy> ");

        add_history(input);

        /* Attempt to parse user input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            lval result = eval(r.output);
            lval_println(result);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise print an error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }
    /* Undefine and delete our parsers */
    mpc_cleanup(6, Number, Double, Symbol, Sexpr, Expr, Lispy);
    return 0;
}

/* POINTERS
 *
 * One reason we need pointers is because of how function calling works. When you call a function in C, you 
 * pass by value, which means you send a copy of that object. So if you are passing around big structs then
 * you will start to use a lot of memory to store these objects. A second problem is when we define a struct,
 * it is always a fixed size, has a limited number of fields and each of these fields must be a struct which
 * itself is limited in size. If you want to call a function with a list of things you need to use pointers.
 * Rather than copying the data itself to a function, we instead copy a number representing the index at
 * where this data starts, the function being called can look up any amount of data it wants. By using
 * addresses instead of actual data, we can allow a function to access and modify some location in memory
 * without having to copy any data. A pointer is just a number. A number representing the starting index of
 * some data in memory. If we keep track of number of bytes needed, we can create variable sized data
 * structures. We declare pointer types by suffixing existing ones with the * character. To create a pointer
 * to some data, we need to get its index, or address. We use the address operator '&'. Dereferencing a
 * pointer, we use the '*' operator on the left-hand side of a variable. To get the data at the field of a
 * pointer to a struct we use the arrow.
 *
 * THE STACK
 *
 * The stack is the memory where your program lives. It is where all of your temporary variables and data
 * structures exist as you manipulate and edit them. Every time you call a function a new area of the stack
 * is put aside for it to use. When the function is done using this area, it is unallocated and free for use.
 *
 * THE HEAP
 *
 * The heap is a section of memory put aside for storage of objects with a longer lifespan. Memory in this
 * area has to be manually allocated and deallocated. To allocate new memory we use the `malloc` function. The
 * function takes as input the number of bytes required, and returns back a pointer to a new block of memory
 * with that many bytes set aside. When done with the memory, we use the `free` function passing it the
 * pointer we received from the `malloc` function. Using the heap you have to remember to call free otherwise
 * the program will continuously allocate more and more memory (a memory leak).
 */
