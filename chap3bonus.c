#include <stdio.h>

void print_hello_world();
void while_hello_world();
void do_while_loop();

void print_hello_world_n_times(int n)
{
    for (int i = 0; i < n; i++) {
        printf("0Hello, world\n");
    }
}

int main(int argc, char *argv[])
{
   print_hello_world();
   while_hello_world();
   print_hello_world_n_times(10);
   do_while_loop();
   return 0;
}

void print_hello_world()
{
    for (int i = 0; i < 5; i++) {
        printf("1Hello, world\n");
    }
}

void while_hello_world()
{
    int i = 5;
    while (i > 0) {
        printf("2Hello, world\n");
        i--;
    }
}

void do_while_loop()
{
    int i = 0;
    do
    {
        printf("3Hello, world\n");
        i++;
    } while (i < 5);
}

