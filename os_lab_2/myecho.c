#include <stdio.h>
#include <stdlib.h>

/* Выводит аргументы командной строки */

int main(int argc, char *argv[])
{
    for (size_t j = 0; j < argc; j++)
        printf("argv[%zu]: %s\n", j, argv[j]);

    exit(EXIT_SUCCESS);
}
