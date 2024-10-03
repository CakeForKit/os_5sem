#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Ожидание процессом - предком завершения потомков. Системный вызов wait(). */

int main(void)
{
    pid_t chpids[2];
    int wstatus;
    pid_t w;

    char *progs[2] = {"./sum.exe", "./concat_nums.exe"};
    static char *newargv[] = { NULL, NULL };

    for (size_t i = 0; i < 2; ++i)
    {
        if ((chpids[i] = fork()) == -1)
        {
            perror("Can not fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (chpids[i] == 0)
        {
            printf("Child process:\tPID = %d,\tPPID = %d,\tGROUP = %d\n",
                   getpid(), getppid(), getpgrp());   

            newargv[0] = progs[i];
            if ((execv(progs[i], newargv)) == -1) {
                perror("execve");   /* execve() returns only on error */
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            printf("\nParent process:\tPID = %d,\tGROUP = %d,\tchpids[%ld] = %d\n",
                   getpid(), getpgrp(), i, chpids[i]);

            
        }
    }

    for (int i = 0; i < 2; ++i) {
        w = wait(&wstatus); // until one of its children terminates (ANY child process.)
        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        printf("terminated pid = %d\t", w);
        if (WIFEXITED(wstatus)) {                                   
            printf("exited, status = %d\n", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d\n", WTERMSIG(wstatus));
        } 
    }

    exit(EXIT_SUCCESS);
}