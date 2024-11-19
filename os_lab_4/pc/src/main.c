

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
#define SHOWBUF 1
#define MAX_TIME_SLEEP 1

#define N_BUF 10
struct buf_struct {
    size_t prod_pos;
    size_t cons_pos;
    char sym_now;
    char buf[N_BUF];
};
typedef struct buf_struct buf_t;

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

void producer(const int semid, buf_t * const buf) {
    char sym_next;
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pprod, 2) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            errExit(err_msg);
        }
        
        sym_next = 'a' + (char)((buf->sym_now - 'a' + 1) % 26);
        buf->buf[buf->prod_pos] = sym_next;
        buf->prod_pos = (buf->prod_pos + 1) % N_BUF;
        buf->sym_now = sym_next;
        printf("Производитель PID=%d положил '%c'", getpid(), sym_next);
        if (SHOWBUF) {
            printf("\t");
            for (size_t i = 0; i < N_BUF; ++i)
                printf("%c", buf->buf[i]);
        }
        printf("\n");

        if (semop(semid, Vprod, 2) == -1)
        errExit("semop");
    }
    exit(EXIT_SUCCESS);
}

void consumer(const int semid, buf_t * const buf) {
    char symb;
    size_t tmp;
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        if (semop(semid, Pcons, 2) == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
            errExit(err_msg);
        }

        tmp = buf->cons_pos;
        symb = buf->buf[buf->cons_pos];
        buf->cons_pos = (buf->cons_pos + 1) % N_BUF;
        printf("Потребитель   PID=%d взял    '%c'", getpid(), symb);
        if (SHOWBUF) {
            buf->buf[tmp] = '-';
            printf("\t");
            for (size_t i = 0; i < N_BUF; ++i)
                printf("%c", buf->buf[i]);
            printf("\n");
        }

        if (semop(semid, Vcons, 2) == -1)
            errExit("semop");
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int semid, shmid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    buf_t *buf;
    int cpid;
    int wstatus;
    pid_t w, chpid[PROD_CNT + CONS_CNT];
    key_t key;

    if (signal(SIGINT, handler) == SIG_ERR) 
        errExit("signal");

    key = ftok(argv[0], 1);
    if (key == -1)
        errExit("ftok");

    shmid = shmget(key, sizeof(buf_t), IPC_CREAT | perms);
    if (shmid == -1) 
        errExit("shmget");
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1)
        errExit("shmat");
    buf->prod_pos = 0;
    buf->cons_pos = 0;
    buf->sym_now = 'a';
    if (SHOWBUF) {
        for (size_t i = 0; i < N_BUF; ++i) {
            buf->buf[i] = '-';
        }
    }

    semid = semget(key, 3, IPC_CREAT | perms);
    if (semid == -1) 
        errExit("semget");
    if (semctl(semid, BINARY, SETVAL, 1) == -1 || 
            semctl(semid, BEMPTY, SETVAL, N_BUF) == -1 || 
            semctl(semid, BFULL, SETVAL, 0) == -1)
        errExit("semctl");

    for (size_t i = 0; i < PROD_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
        {
            chpid[i] = cpid;
            producer(semid, buf);
            exit(0);
        }
    }
    for (size_t i = 0; i < CONS_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
        {
            chpid[i + PROD_CNT] = cpid;
            consumer(semid, buf);
            exit(0);
        }
    }

    if (signal(SIGINT, handler) == SIG_ERR) 
        errExit("signal");

    for (size_t i = 0; i < PROD_CNT + CONS_CNT; ++i) {
        w = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED); // until one of its children terminates (ANY child process.)
        if (w == -1) 
            errExit("wait");

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
    if (shmdt(buf) == -1)
        errExit("shmdt");
    if (semctl(semid, 0, IPC_RMID) == -1)
        errExit("semctl");
    
    return 0;
}