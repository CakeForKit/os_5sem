#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/* Получение сироты */

int main(void)
{
    pid_t chld1, chld2;

    if ((chld1 = fork()) == -1)
    {
        perror("Can not fork.\n");
        exit(EXIT_FAILURE);
    }
    else if (chld1 == 0)
    {
        printf("Child process 1:\tPID = %d,\tPPID = %d,\tGROUP = %d\n",
                getpid(), getppid(), getpgrp());

        if ((chld2 = fork()) == -1)
        {
            perror("Can not fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (chld2 == 0)
        {
            printf("Child child process 2:\tPID = %d,\tPPID = %d,\tGROUP = %d\n",
                getpid(), getppid(), getpgrp());
        }
    }
    else
    {
        printf("Parent process 1:\tPID = %d,\tGROUP = %d,\tchld1 = %d\n",
                getpid(), getpgrp(), chld1);
    }

    while(1);

    return 0;
}