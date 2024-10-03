#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
int execve(const char *pathname, char *const _Nullable argv[], char *const _Nullable envp[]);
    - executes the program referred to by pathname
    - argv is an array of pointers to strings passed to the new program as its command-line arguments.
    - envp is an array of pointers to strings, conventionally of the
       form key=value, which are passed as the environment of the new
       program.  The envp array must be terminated by a null pointer.
*/

int main(int argc, char *argv[])
{
    static char *newargv[] = { NULL, "hello", "world", NULL };  // The argv array must be terminated by a null pointer. 
    static char *newenviron[] = { NULL };

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file-to-exec>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    newargv[0] = argv[1];   //  By convention, the first of these
                            //  strings (i.e., argv[0]) should contain the filename associated
                            //  with the file being executed.

    execve(argv[1], newargv, newenviron);
    perror("execve");   /* execve() returns only on error */
    exit(EXIT_FAILURE);
}

