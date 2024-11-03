#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int flag = 0;

void handler(int sig_numb)
{
    flag = 1;
    printf(" Signal handler %d installed.\n", sig_numb);
}

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

    if (signal(SIGINT, handler) == SIG_ERR) {
        perror("Ошибка signal");
        exit(EXIT_FAILURE);
    }
    printf("Push Ctrl+C, to send message from childs.\n");
    sleep(2);

    for (size_t i = 0; i < 2; ++i) {
        if ((chpids[i] = fork()) == -1) {
            perror("Can not fork.\n");
            exit(EXIT_FAILURE);
        }
        else if (chpids[i] == 0) {
            
            if (flag) {
                printf("PID = %d write: '%s'\n", getpid(), texts[i]);
                close(pipefd[0]);                               
                write(pipefd[1], texts[i], strlen(texts[i])); 
            } else 
                printf("PID = %d no signal \n", getpid());
                                         
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
    printf("\n");

    char buf[50];
    for (size_t i = 0; i < 2; ++i) {
        close(pipefd[1]);          
        read(pipefd[0], &buf, strlen(texts[i]));
        printf("Received message from PID = %d: %s\n", chpids[i], buf);
        buf[0] = 0;
    }
    close(pipefd[1]); 
    read(pipefd[0], buf, 1);
    printf("Received message: %s\n", buf);
    
    exit(EXIT_SUCCESS);
}