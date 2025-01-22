#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    pid_t chpids[2];
    pid_t w;
    char *texts[2] = {"xx", "y-y-y-y-y"};
    int pipefd[2], wstatus;
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < 2; ++i) {
        if ((chpids[i] = fork()) == -1) {
            perror("Can not fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (chpids[i] == 0) {
            printf("PID = %d write: '%s'\n", getpid(), texts[i]);
            close(pipefd[0]);                               
            write(pipefd[1], texts[i], strlen(texts[i]));
            close(pipefd[1]);                               
            exit(EXIT_SUCCESS);
        }
    }
    for (size_t i = 0; i < 2; ++i) {
        w = wait(&wstatus); 
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
        close(pipefd[1]);          
        read(pipefd[0], &buf, strlen(texts[i]));
        printf("Parant received message from PID = %d: %s\n", chpids[i], buf);
        buf[0] = 0;
    }
    close(pipefd[1]); 
    read(pipefd[0], buf, 1);
    printf("received message: '%s'\n", buf);
    close(pipefd[0]);  
    exit(EXIT_SUCCESS);
}