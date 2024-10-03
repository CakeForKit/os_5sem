#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/* Получение сироты */

int main(void)
{
    pid_t chpids[2];

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

            sleep(2);
            // К этому поменту parant уже завершится и процесс станет сиротой
            printf("\nChild process:\tPID = %d,\tPPID = %d,\tGROUP = %d\n",
                   getpid(), getppid(), getpgrp());     
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Parent process:\tPID = %d,\tGROUP = %d,\tchpids[%ld] = %d\n",
                   getpid(), getpgrp(), i, chpids[i]);
        }
    }

    return 0;
}