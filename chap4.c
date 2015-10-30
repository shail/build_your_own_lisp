 /*REPL for our programming language*/
 /*This will be an introduction where we just prompt the user and echo back their input*/
/*Have to run this file with flag -ledit*/
/*#ifdef _WIN32 #else #endif -- this is to check whether something is WIN32 and do something different if that
 * is the underlying architecture*/
/*ifndef macro -- if macro has been defined using a #define statement, then the code will be compile*/
/*define CNAME value or define CNAME (expression)*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <editline/readline.h>

/* Declare a buffer for user input of size 2048
 * This is a reserved block of data we can access anywhere from our program. static keyword makes this
 * variable local to this file */
/*static char input[2048];*/

int main(int argc, char** argv)
{
    puts("Sammy Version 0.0.0.0.1");
    puts("Press Ctrl+c to Exit\n");

    while (1) {
        /* Output our prompt. This is a slight variation on puts as this doesn't append a newline char */
        /*fputs("lispy> ", stdout);*/
        /*readline gets the input in one line and then we pass the result to the history, readline
         * strips the trailing newline character from the input, we need to delete the input given to
         * us from the readline function*/
        char *input = readline("sammy> ");

        /*Add input to history*/
        input[strlen(input)] = 'b';
        add_history(input);

        /* Read a line of user input of maximum size 2048. This gets user input, reading from stdin
         * which is defined in stdio.h, fgets will wait for a user to input a line of text which it
         * will store in input, including the newline character, also need to supply buffer size*/
        /*fgets(input, 2048, stdin);*/

        /* Echo input back to user */
        printf("No you're a %s\n", input);

        /*Free retrieved input*/
        free(input);
    }

    return 0;
}
