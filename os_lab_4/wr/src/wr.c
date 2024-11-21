#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)
#define MAX_TIME_SLEEP 1

#define WRITERS_CNT     4   // 3
#define READERS_CNT     5   // 5

#define FLAG_EDIT       0
#define NUMB_OF_READERS  1
#define WRITES_QUEUE    2
#define READERS_QUEUE   3

struct sembuf start_r[] = {
    {FLAG_EDIT, 0, 0},
    {WRITES_QUEUE, 0, 0},
    {NUMB_OF_READERS, 1, 0},
    {READERS_QUEUE, -1, 0}
};
struct sembuf Vreaders_queue[] = { {READERS_QUEUE, 1, 0} };
struct sembuf stop_r[] = { {NUMB_OF_READERS, -1, 0} };

void start_read(const int semid) {
    if (semop(semid, Vreaders_queue, 1) == -1) 
        errExit("semop");
    if (semop(semid, start_r, 4) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop(start_r) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        errExit(err_msg);
    }
}

void stop_read(const int semid) {
    if (semop(semid, stop_r, 1) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        errExit(err_msg);
    }
}

struct sembuf start_w[] = {
    {NUMB_OF_READERS, 0, 0},
    {FLAG_EDIT, 0, 0},
    {FLAG_EDIT, 1, 0},
    {WRITES_QUEUE, -1, 0}
};
struct sembuf Vwriters_queue[] = {{WRITES_QUEUE, 1, 0}};
struct sembuf stop_w[] = { {FLAG_EDIT, -1, 0} };

void start_write(const int semid) {
    if (semop(semid, Vwriters_queue, 1) == -1) 
        errExit("semop");
    if (semop(semid, start_w, 4) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop(start_w) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        errExit(err_msg);
    }
}

void stop_write(const int semid) {
    if (semop(semid, stop_w, 1) == -1)
        errExit("semop");
}

int f_sigint = 1;

void handler(int sig_numb) {
    f_sigint = 0;
}

void reader(const int semid, char * const buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));

        start_read(semid);
        printf("Читатель PID=%d считал '%c'\n", getpid(), *buf);
        stop_read(semid);
    }
    exit(EXIT_SUCCESS);
}

void writer(const int semid, char * const buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));
        
        start_write(semid);
        if (*buf == 'z')
            *buf = 'a';
        else
            (*buf)++;
        // sleep(1);
        printf("Писатель PID=%d записал '%c'\n", getpid(), *buf);
        stop_write(semid);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    int semid, shmid;
    int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    char *buf;
    int cpid, wstatus;
    pid_t w;
    key_t key;

    if (signal(SIGINT, handler) == SIG_ERR) 
        errExit("signal");

    key = ftok(argv[0], 1);
    if (key == -1)
        errExit("ftok");

    shmid = shmget(key, 1, IPC_CREAT | perms);
    if (shmid == -1) 
        errExit("shmget");
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1)
        errExit("shmat");
    *buf = 'a';

    semid = semget(key, 4, IPC_CREAT | perms);
    if (semid == -1) 
        errExit("semget");
    if (semctl(semid, FLAG_EDIT, SETVAL, 0) == -1 || 
            semctl(semid, NUMB_OF_READERS, SETVAL, 0) == -1 || 
            semctl(semid, WRITES_QUEUE, SETVAL, 0) == -1 ||
            semctl(semid, READERS_QUEUE, SETVAL, 0) == -1)
        errExit("semctl");

    for (size_t i = 0; i < WRITERS_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
            writer(semid, buf);
    }
    for (size_t i = 0; i < READERS_CNT; ++i) {
        if ((cpid = fork()) == -1)
            errExit("fork");
        else if (cpid == 0)
            reader(semid, buf);
    }

    for (size_t i = 0; i < WRITERS_CNT + READERS_CNT; ++i) {
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