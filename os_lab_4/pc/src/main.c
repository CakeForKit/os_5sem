

#include "consts.h"
#include "prod_cons.h"
#include <signal.h>

void handlerkill(int sig_numb)
{
    exit(EXIT_SUCCESS);
}

int main(void) {
    int semid, shmid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    buf_t *buf;
    int cpid;
    int wstatus;
    pid_t w;

    shmid = shmget(IPC_PRIVATE, sizeof(buf_t), IPC_CREAT | IPC_EXCL | perms);
    if (shmid == -1) 
        errExit("shmget");
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1)
        errExit("shmat");
    buf->prod_pos = 0;
    buf->cons_pos = 0;
    buf->sym_now = 'a';
    if (DEBUG) {
        for (size_t i = 0; i < N_BUF; ++i) {
            buf->buf[i] = '-';
        }
    }

    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
    if (semid == -1) 
        errExit("semget");
    if (semctl(semid, BINARY, SETVAL, 1) == -1)
        errExit("semctl");
    if (semctl(semid, BEMPTY, SETVAL, N_BUF) == -1)
        errExit("semctl");
    if (semctl(semid, BFULL, SETVAL, 0) == -1)
        errExit("semctl");

    if (signal(SIGINT, handlerkill) == SIG_ERR) 
        errExit("signal");

    for (size_t i = 0; i < PROD_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
        {
            producer(semid, buf);
            exit(0);
        }
    }
    for (size_t i = 0; i < CONS_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
        {
            consumer(semid, buf);
            exit(0);
        }
    }

    for (size_t i = 0; i < PROD_CNT + CONS_CNT; ++i) {
        w = wait(&wstatus); // until one of its children terminates (ANY child process.)
        if (w == -1) 
            errExit("wait");

        printf("PID=%d ", w);
        if (WIFEXITED(wstatus)) {
            printf("exited, status=%d\n", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d\n", WTERMSIG(wstatus));
        } 
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl");
    if (shmdt(buf) == -1)
        errExit("shmdt");
    if (semctl(semid, 0, IPC_RMID) == -1)
        errExit("semctl");
    
    return 0;
}