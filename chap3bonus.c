#include <stdio.h>

void print_hello_world();
void while_hello_world();

void print_hello_world_n_times(int n)
{
    for (int i = 0; i < n; i++) {
        printf("Hello, world\n");
    }
}

int main(int argc, char *argv[])
{
   print_hello_world(); 
   while_hello_world();
   print_hello_world_n_times(10);
}

void print_hello_world()
{
    for (int i = 0; i < 5; i++) {
        printf("Hello, world\n");
    }
}

void while_hello_world()
{
    int i = 5;
    while (i > 0) {
        printf("Hello, world\n");
        i--;
    }
}
