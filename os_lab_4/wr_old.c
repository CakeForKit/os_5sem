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
#define READERS_CNT     6   // 5

#define FLAG_EDIT       0
#define NUMB_OF_READERS  1
#define WRITERS_QUEUE    2
#define READERS_QUEUE   3

struct sembuf sem_start_read[] = {
    {READERS_QUEUE, 1, 0},
    {FLAG_EDIT, 0, 0},
    {WRITERS_QUEUE, 0, 0},
    {NUMB_OF_READERS, 1, 0},
    {READERS_QUEUE, -1, 0}
};
struct sembuf sem_stop_read[] = { {NUMB_OF_READERS, -1, 0} };

struct sembuf sem_start_write[] = {
    {WRITERS_QUEUE, 1, 0},
    {NUMB_OF_READERS, 0, 0},
    {FLAG_EDIT, 0, 0},
    {FLAG_EDIT, 1, 0},
    {WRITERS_QUEUE, -1, 0}
};
struct sembuf sem_stop_write[] = { {FLAG_EDIT, -1, 0} };

void start_read(int semid) {
    if (semop(semid, sem_start_read, 5) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop(sem_start_read) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        perror(err_msg); 
        exit(EXIT_FAILURE);
    }
}

void stop_read(int semid) {
    if (semop(semid, sem_stop_read, 1) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        perror(err_msg); 
        exit(EXIT_FAILURE);
    }
}

void start_write(int semid) {
    if (semop(semid, sem_start_write, 5) == -1) {
        char err_msg[100];
        sprintf(err_msg, "ERR: semop(sem_start_write) PID=%d, errno=%d (EINTR=%d)", getpid(), errno, EINTR);
        perror(err_msg); 
        exit(EXIT_FAILURE);
    }
}

void stop_write(int semid) {
    if (semop(semid, sem_stop_write, 1) == -1){
        perror("semop"); 
        exit(EXIT_FAILURE);
    }
}

int f_sigint = 1;

void handler(int sig_numb) {
    printf("PID=%d signal=%d\n", getpid(),sig_numb);
    f_sigint = 0;
}

void reader(int semid, char *buf) {
    srand(time(NULL));
    while(f_sigint) {
        sleep(rand() % (MAX_TIME_SLEEP + 1));

        start_read(semid);
        printf("Читатель PID=%d считал '%c'\n", getpid(), *buf);
        stop_read(semid);
    }
    exit(EXIT_SUCCESS);
}

void writer(int semid, char *buf) {
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

    if (signal(SIGINT, handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }

    key = ftok(argv[0], 1);
    if (key == -1){
        perror("ftok"); 
        exit(EXIT_FAILURE);
    }

    shmid = shmget(key, 1, IPC_CREAT | perms);
    if (shmid == -1) {
        perror("shmid"); 
        exit(EXIT_FAILURE);
    }
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1){
        perror("shmat"); 
        exit(EXIT_FAILURE);
    }
    *buf = 'a';

    semid = semget(key, 4, IPC_CREAT | perms);
    if (semid == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, FLAG_EDIT, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, NUMB_OF_READERS, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, WRITERS_QUEUE, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, READERS_QUEUE, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < WRITERS_CNT; ++i) {
        if ((cpid = fork()) == -1){
            perror("fork"); 
            exit(EXIT_FAILURE);
        }
        else if (cpid == 0)
            writer(semid, buf);
    }
    for (size_t i = 0; i < READERS_CNT; ++i) {
        if ((cpid = fork()) == -1) {
            perror("ftok"); 
            exit(EXIT_FAILURE);
        }
        else if (cpid == 0)
            reader(semid, buf);
    }

    for (size_t i = 0; i < WRITERS_CNT + READERS_CNT; ++i) {
        w = waitpid(-1, &wstatus, WUNTRACED | WCONTINUED); // until one of its children terminates (ANY child process.)
        if (w == -1) {
            char err_msg[100];
            sprintf(err_msg, "ERR: waitpid PID=%d, errno=%d", getpid(), errno);
            perror(err_msg); 
            exit(EXIT_FAILURE);
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

    if (shmdt(buf) == -1) {
        perror("shmdt"); 
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("shmctl"); 
        exit(EXIT_FAILURE);
    }
    
    return 0;
}