#include "mpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <editline/readline.h>

/* Create Enumeration of Possible lval Types */
typedef enum { LVAL_NUM, LVAL_DUB, LVAL_SYM, LVAL_SEXPR, LVAL_ERR } lval_type_t;

/* S-expressions are variable length lists. We create a pointer field cell which points to a locatino where
 * we store a list of lval*. Therefore this is a double pointer list, where we have a pointer to lval
 * pointers. We also have to keep track of how many lval pointers are in the list. If we name a struct
 * we can refer to its own type, but can only contain pointers to its own type. Otherwise the size of
 * the struct would refer to itself, and grow infinite in size when you tried to calculate it*/
typedef struct lval {
    lval_type_t type;
    union {
        long num;
        double doub;
    } val;
    /* Error and Symbol types have some string data */
    char *err;
    char *sym;
    /* Count and Pointer to a list of "lval*" */
    int count;
    struct lval** cell;
} lval;

/* Construct a pointer to a new Number lval */
lval* lval_num(long x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->val.num = x;
    return v;
}

lval* lval_doub(double x) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_DUB;
    v->val.num = x;
    return v;
}

lval* lval_err(char* m) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) + 1);
    strcpy(v->err, m);
    return v;
}

/* We have to use strlen(s) + 1 because C strings are null terminated, final character is \0 */
lval* lval_sym(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SYM;
    v->sym = malloc(strlen(s) + 1);
    strcpy(v->sym, s);
    return v;
}
/* NULL in this case is saying we have a data pointer but not pointing to any number of data items */
lval* lval_sexpr(void) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_SEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

/* This frees memory acquired by malloc */
void lval_del(lval* v) {
    switch (v->type) {
        case LVAL_NUM: break;
        case LVAL_DUB: break;
        case LVAL_ERR: free(v->err); break;
        case LVAL_SYM: free(v->sym); break;
        case LVAL_SEXPR:
          for(int i = 0; i < v->count; i++) {
              lval_del(v->cell[i]);
          }
          free(v->cell);
        break;
    }
    free(v);
}

lval* lval_read_doub(mpc_ast_t* t) {
   errno = 0;
   double x = strtod(t->contents, NULL);
   return errno != ERANGE ? lval_doub(x) : lval_err("invalid number");
}

lval* lval_add(lval* v, lval* x) {
    v->count++;
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

lval* lval_pop(lval* v, int i) {
    /* Find the item at "i" */
    lval* x = v->cell[i];

    /* Shift memory after the item at "i" over the top */
    memmove(&v->cell[i], &v->cell[i+1],
      sizeof(lval*) * (v->count-i-1));

    /* Decrease the count of items in the list */
    v->count--;

    /* Reallocate the memory used */
    v->cell = realloc(v->cell, sizeof(lval*) * v->count);
    return x;
}

lval* lval_take(lval* v, int i) {
    lval* x = lval_pop(v, i);
    lval_del(v);
    return(x);
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
    putchar(open);
    for (int i = 0; i < v->count; i++) {
        lval_print(v->cell[i]);

        if (i != (v->count-1)) {
            putchar(' ');
        }
    }
    putchar(close);
}

void lval_print(lval* v)
{
    switch (v->type) {
        case LVAL_NUM: printf("%li", v->val.num); break;
        case LVAL_DUB: printf("%f", v->val.doub); break;
        case LVAL_SYM: printf("%s", v->sym); break;
        case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
        case LVAL_ERR: printf("%s", v->err); break;
        break;
    }
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

lval* builtin_op(lval* a, char* op) {
    /* Ensure all arguments are numbers */
    for (int i = 0; i < a->count; i++) {
        if (a->cell[i]->type != LVAL_NUM) {
          lval_del(a);
          return lval_err("Cannot operate on non-number!");
        }
    }

    /* Pop the first element */
    lval* x = lval_pop(a, 0);

    /* If no arguments and sub then perform unary negation */
    if ((strcmp(op, "-") == 0) && a->count == 0) {
        x->val.num = -x->val.num;
    }

    /* While there are still elements remaining */
    while (a->count > 0) {
        /* Pop the next element */
        lval* y = lval_pop(a, 0);

        if (strcmp(op, "+") == 0) { x->val.num += y->val.num; }
        if (strcmp(op, "-") == 0) { x->val.num -= y->val.num; }
        if (strcmp(op, "*") == 0) { x->val.num *= y->val.num; }
        if (strcmp(op, "/") == 0) {
            if (y->val.num == 0) {
                lval_del(x); lval_del(y);
                x = lval_err("Division By Zero!"); break;
            }
            x->val.num /= y->val.num;
        }
        if (strcmp(op, "%") == 0) { x->val.num %= y->val.num; }
        lval_del(y);
    }
    lval_del(a); return x;
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v) {
    /* Evaluate children */
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(v->cell[i]);
    }

    /* Error Checking */
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
    }

    /* Empty Expression */
    if (v->count == 0) { return v; }

    /* Single Expression */
    if (v->count == 1) { return lval_take(v, 0); }

    /* Ensure First Element is Symbol */
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_SYM) {
        lval_del(f); lval_del(v);
        return lval_err("S-expression does not start with symbol!");
    }

    /* Call builtin with operator */
    lval *result = builtin_op(v, f->sym);
    lval_del(f);
    return result;
}

lval* lval_eval(lval* v) {
    /* Evaluate Sexpressions */
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
    /* All other lval types remain the same */
    return v;
}

lval* lval_read_num(mpc_ast_t* t) {
   errno = 0;
   long x = strtol(t->contents, NULL, 10);
   return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
    if (strstr(t->tag, "number")) { return lval_read_num(t); }
    if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

    /* If root (>) or sexpr then create empty list */
    lval* x = NULL;
    if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
    if (strcmp(t->tag, "sexpr") == 0) { x = lval_sexpr(); }

    /* Fill this list with any valid expression contained within */
    for (int i = 0; i< t->children_num; i++) {
        if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
        if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
        if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
        if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
        x = lval_add(x, lval_read(t->children[i]));
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
        symbol   : '+' | '-' | '*' | '/' | '%' ;                                                           \
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
            lval* x = lval_eval(lval_read(r.output));
            lval_println(x);
            lval_del(x);
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

/* EXTRA CREDIT
 * Give an example of a variable in our program that lives on The Stack.
 * long x = strtol(t->contents, NULL, 10); -- line 228 is on the Stack
 *
 * Give an example of a variable in our program that points to The Heap.
 * lval v* in lval_num is on the Heap since the memory is allocated manually.
 *
 * What does the strcpy function do?
 * Copies the string pointed to by the src, including the null terminator, to the buffer pointed to by dest.
 * The destination string must be large enough to receive the copy or there will be a buffer overrun.
 *
 * What does the realloc function do?
 * Resize the memory block pointed to by the ptr argument that was previously allocated with malloc or calloc,
 * providing the pointer to the previously allocated memory block and then the new size, return value is
 * a pointer to the newly allocated memory.
 *
 * What does the memmove function do?
 * Copies n bytes from memory area src to memory area dest, memory areas may overlap, copying takes place
 * as though the bytes in src are first copied to a temporary array that doesn't overlap with src or dest,
 * and the bytes are then copied from the temporary array to dest. Supply two pointers and n number of bytes
 * to be copied. The function returns a pointer to the destination. This is safer than memcpy for overlapping
 * memory because it does a bit more work to make sure it handles overlapping memory.
 */
