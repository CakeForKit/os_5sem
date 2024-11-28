

#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
#define MAX_TIME_SLEEP 1
#define N_BUF 10

#define PROD_CNT 6      // 4
#define CONS_CNT 7     // 5

#define BINARY 0
#define BEMPTY 1
#define BFULL 2
#define P -1
#define V  1
struct sembuf Pprod[2] = {{BEMPTY, P, SEM_UNDO}, {BINARY, P, SEM_UNDO}};
struct sembuf Vprod[2] = {{BINARY, V, SEM_UNDO}, {BFULL, V, SEM_UNDO}};
struct sembuf Pcons[2] = {{BFULL, P, SEM_UNDO}, {BINARY, P, SEM_UNDO}};
struct sembuf Vcons[2] = {{BINARY, V, SEM_UNDO}, {BEMPTY, V, SEM_UNDO}};

int f_sigint = 1;

void handler(int sig_numb) {
    f_sigint = 0;
}

void producer(const int semid, char** prod_ptr, char* alfa, char *buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pprod, 2) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            errExit(err_msg);
        }
        if ((*alfa) - 1 == 'z')
            *alfa = 'a';
        (*prod_ptr)++;
        *(*prod_ptr) = (*alfa)++;
        printf("Производитель PID=%d положил '%c'", getpid(), *alfa);

        for (char *c = buf; c < buf + N_BUF; ++c) {
            printf("%c", c);
        }
        printf("\n");

        if (semop(semid, Vprod, 2) == -1)
        errExit("semop");
    }
    exit(EXIT_SUCCESS);
}

void consumer(const int semid, char** cons_ptr, char* alfa, , char *buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pcons, 2) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            errExit(err_msg);
        }
        *(alfa - 1) = '-';
        (*cons_ptr)++;
        printf("Потребитель   PID=%d взял    '%c'", getpid(), *(*cons_ptr));

        for (char *c = buf; c < buf + N_BUF; ++c) {
            printf("%c", c);
        }
        printf("\n");

        if (semop(semid, Vcons, 2) == -1)
            errExit("semop");
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int semid, shmid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char** prod_ptr;
    char** cons_ptr;
    char* alfa;
    char *addr;
    int cpid, wstatus;
    pid_t w;
    key_t key;

    if (signal(SIGINT, handler) == SIG_ERR) 
        errExit("signal");

    key = ftok(argv[0], 1);
    if (key == -1)
        errExit("ftok");

    shmid = shmget(key, N_BUF + 3, IPC_CREAT | perms);
    if (shmid == -1) 
        errExit("shmget");
    addr = shmat(shmid, NULL, 0);
    if (addr == (void *) -1)
        errExit("shmat");

    prod_ptr = (char **) addr;
    cons_ptr = prod_ptr + sizeof(char *);
    alfa = (char *)(cons_ptr + sizeof(char));
    *cons_ptr = alfa + sizeof(char);
    *prod_ptr = *cons_ptr;
    *alfa = 'a';

    semid = semget(key, 3, IPC_CREAT | perms);
    if (semid == -1) 
        errExit("semget");
    if (semctl(semid, BINARY, SETVAL, 1) == -1)
        errExit("semctl");
    if (semctl(semid, BEMPTY, SETVAL, N_BUF) == -1)
        errExit("semctl");
    if (semctl(semid, BFULL, SETVAL, 0) == -1)
        errExit("semctl");

    for (size_t i = 0; i < PROD_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
            producer(semid, prod_ptr, alfa);
    }
    for (size_t i = 0; i < CONS_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
            consumer(semid, cons_ptr);
    }

    for (size_t i = 0; i < PROD_CNT + CONS_CNT; ++i) {
        w = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED); // until one of its children terminates (ANY child process.)
        if (w == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: waitpid PID=%d, errno=%d", getpid(), errno);
            errExit(err_msg);
        }


        printf("PID=%d ", w);
        if (WIFEXITED(wstatus)) {                                    
            printf("exited, status = %d ", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d ", WTERMSIG(wstatus));
        } else if (WIFSTOPPED(wstatus)) {
            printf("stopped by signal %d ", WSTOPSIG(wstatus));
        } else if (WIFCONTINUED(wstatus)) {
            printf("continued ");
        }
        printf("\n");
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl");
    if (shmdt(addr) == -1)
        errExit("shmdt");
    if (semctl(semid, 0, IPC_RMID) == -1)
        errExit("semctl");
    
    return 0;
}