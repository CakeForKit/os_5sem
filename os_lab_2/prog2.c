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

            if (i == 1)
                while (1)
                    ;
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("\nParent process:\tPID = %d,\tGROUP = %d,\tchpids[%ld] = %d\n",
                   getpid(), getpgrp(), i, chpids[i]);

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
            else if (WIFSTOPPED(wstatus)) {
                printf("stopped by signal %d\n", WSTOPSIG(wstatus));
            } else if (WIFCONTINUED(wstatus)) {
                printf("continued\n");
            }
            /*
            WIFEXITED(wstatus)      returns true if the child terminated normally, that is, by
                                    calling exit(3) or _exit(2), or by returning from main().
            WIFSIGNALED(wstatus)    returns true if the child process was terminated by a signal.
            WIFSTOPPED(wstatus)     returns true if the child process was stopped by delivery
                                    of a signal; this is possible only if the call was done
                                    using WUNTRACED or when the child is being traced (see
                                    ptrace(2)).
            WIFCONTINUED(wstatus)   (since Linux 2.6.10) returns true if the child process was
                                    resumed by delivery of SIGCONT.
                                    SIGCONT — сигнал, посылаемый для возобновления выполнения 
                                    процесса, ранее остановленного сигналом SIGSTOP или другим сигналом
              */
        }
    }

    return 0;
}