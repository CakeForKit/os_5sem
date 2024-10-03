#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

/* Ожидание процессом - предком завершения потомков. Системный вызов wait(). */

int main(void)
{
    pid_t chpids[2];
    int wstatus;
    pid_t w;

    char *texts[2] = {"xx", "y-y-y-y-y"};

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

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

            printf("child[%ld] write: '%s'\n", i, texts[i]);
            close(pipefd[0]);          /* Close unused read end */
            write(pipefd[1], texts[i], strlen(texts[i]));
            close(pipefd[1]);          /* Reader will see EOF */
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("\nParent process:\tPID = %d,\tGROUP = %d,\tchpids[%ld] = %d\n",
                   getpid(), getpgrp(), i, chpids[i]);
        }
    }

    for (size_t i = 0; i < 2; ++i) {
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

    char buf[50];
    for (size_t i = 0; i < 2; ++i) {
        close(pipefd[1]);          /* Close unused write end */
        read(pipefd[0], &buf, strlen(texts[i]));
        printf("Parant got text from [%ld]: %s\n", i, buf);
        buf[0] = 0;
    }
    close(pipefd[0]);  

    exit(EXIT_SUCCESS);
}